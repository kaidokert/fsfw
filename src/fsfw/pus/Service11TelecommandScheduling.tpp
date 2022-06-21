#pragma once

#include <cstddef>

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serialize/SerializeAdapter.h"
#include "fsfw/serviceinterface.h"
#include "fsfw/tmtcservices/AcceptsTelecommandsIF.h"

static constexpr auto DEF_END = SerializeIF::Endianness::BIG;

template <size_t MAX_NUM_TCS>
inline Service11TelecommandScheduling<MAX_NUM_TCS>::Service11TelecommandScheduling(
    object_id_t objectId, uint16_t apid, uint8_t serviceId, AcceptsTelecommandsIF *tcRecipient,
    uint16_t releaseTimeMarginSeconds, bool debugMode)
    : PusServiceBase(objectId, apid, serviceId),
      RELEASE_TIME_MARGIN_SECONDS(releaseTimeMarginSeconds),
      debugMode(debugMode),
      tcRecipient(tcRecipient) {}

template <size_t MAX_NUM_TCS>
inline Service11TelecommandScheduling<MAX_NUM_TCS>::~Service11TelecommandScheduling() = default;

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::handleRequest(
    uint8_t subservice) {
  if (debugMode) {
    FSFW_LOGI("PUS11::handleRequest: Handling request {}\n", static_cast<int>(subservice));
  }
  // Get de-serialized Timestamp
  const uint8_t *data = currentPacket.getApplicationData();
  size_t size = currentPacket.getApplicationDataSize();
  if (data == nullptr) {
    return handleInvalidData("handleRequest");
  }
  switch (subservice) {
    case Subservice::ENABLE_SCHEDULING: {
      schedulingEnabled = true;
      break;
    }
    case Subservice::DISABLE_SCHEDULING: {
      schedulingEnabled = false;
      break;
    }
    case Subservice::RESET_SCHEDULING: {
      return handleResetCommand();
    }
    case Subservice::INSERT_ACTIVITY:
      return doInsertActivity(data, size);
    case Subservice::DELETE_ACTIVITY:
      return doDeleteActivity(data, size);
    case Subservice::FILTER_DELETE_ACTIVITY:
      return doFilterDeleteActivity(data, size);
    case Subservice::TIMESHIFT_ACTIVITY:
      return doTimeshiftActivity(data, size);
    case Subservice::FILTER_TIMESHIFT_ACTIVITY:
      return doFilterTimeshiftActivity(data, size);
    default:
      return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
  }
  return RETURN_OK;
}

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::performService() {
  if (not schedulingEnabled) {
    return RETURN_OK;
  }
  // get current time as UNIX timestamp
  timeval tNow = {};
  Clock::getClock_timeval(&tNow);

  // TODO: Optionally limit the max number of released TCs per cycle?
  // NOTE: The iterator is increased in the loop here. Increasing the iterator as for-loop arg
  // does not work in this case as we are deleting the current element here.
  for (auto it = telecommandMap.begin(); it != telecommandMap.end();) {
    if (it->first <= tNow.tv_sec) {
      if (schedulingEnabled) {
        // release tc
        TmTcMessage releaseMsg(it->second.storeAddr);
        auto sendRet = this->requestQueue->sendMessage(recipientMsgQueueId, &releaseMsg, false);

        if (sendRet != HasReturnvaluesIF::RETURN_OK) {
          return sendRet;
        }
        if (debugMode) {
          FSFW_LOGIT("Released TC & erased it from TC map\n");
        }
        telecommandMap.erase(it++);
      } else if (deleteExpiredTcWhenDisabled) {
        telecommandMap.erase(it++);
      }
      continue;
    }
    it++;
  }

  return HasReturnvaluesIF::RETURN_OK;
}

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::initialize() {
  ReturnValue_t res = PusServiceBase::initialize();
  if (res != HasReturnvaluesIF::RETURN_OK) {
    return res;
  }

  tcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::TC_STORE);
  if (!tcStore) {
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }

  if (tcRecipient == nullptr) {
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }
  recipientMsgQueueId = tcRecipient->getRequestQueue();

  return res;
}

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::handleResetCommand() {
  for (auto it = telecommandMap.begin(); it != telecommandMap.end(); it++) {
    ReturnValue_t result = tcStore->deleteData(it->second.storeAddr);
    if (result != RETURN_OK) {
      // This should not happen
      FSFW_LOGW("Service11TelecommandScheduling::handleRequestDeleting: Deletion failed\n");
      triggerEvent(TC_DELETION_FAILED, (it->second.requestId >> 32) & 0xffffffff,
                   it->second.requestId & 0xffffffff);
    }
  }
  telecommandMap.clear();
  return RETURN_OK;
}

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::doInsertActivity(
    const uint8_t *data, size_t size) {
  uint32_t timestamp = 0;
  ReturnValue_t result = SerializeAdapter::deSerialize(&timestamp, &data, &size, DEF_END);
  if (result != RETURN_OK) {
    return result;
  }

  // Insert possible if sched. time is above margin
  // (See requirement for Time margin)
  timeval tNow = {};
  Clock::getClock_timeval(&tNow);
  if (timestamp - tNow.tv_sec <= RELEASE_TIME_MARGIN_SECONDS) {
    FSFW_LOGW("Service11TelecommandScheduling::doInsertActivity: "
        "Release time too close to current time\n");
    return RETURN_FAILED;
  }

  // store currentPacket and receive the store address
  store_address_t addr{};
  if (tcStore->addData(&addr, data, size) != RETURN_OK ||
      addr.raw == storeId::INVALID_STORE_ADDRESS) {
    FSFW_LOGET("Service11TelecommandScheduling::doInsertActivity: Adding data to TC Store failed\n");
    return RETURN_FAILED;
  }

  // insert into multimap with new store address
  TelecommandStruct tc;
  tc.seconds = timestamp;
  tc.storeAddr = addr;
  tc.requestId =
      getRequestIdFromDataTC(data);  // TODO: Missing sanity check of the returned request id

  auto it = telecommandMap.insert(std::pair<uint32_t, TelecommandStruct>(timestamp, tc));
  if (it == telecommandMap.end()) {
    return RETURN_FAILED;
  }

  if (debugMode) {
    FSFW_LOGI("PUS11::doInsertActivity: Inserted into Multimap:\n");
    debugPrintMultimapContent();
  }
  return HasReturnvaluesIF::RETURN_OK;
}

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::doDeleteActivity(
    const uint8_t *data, size_t size) {
  // Get request ID
  uint64_t requestId;
  ReturnValue_t result = getRequestIdFromData(data, size, requestId);
  if (result != RETURN_OK) {
    return result;
  }

  // DEBUG
  if (debugMode) {
    FSFW_LOGI("PUS11::doDeleteActivity: requestId: {}\n", requestId);
  }

  TcMapIter tcToDelete;     // handle to the TC to be deleted, can be used if counter is valid
  int tcToDeleteCount = 0;  // counter of all found TCs. Should be 1.

  for (auto it = telecommandMap.begin(); it != telecommandMap.end(); it++) {
    if (it->second.requestId == requestId) {
      tcToDelete = it;
      tcToDeleteCount++;
    }
  }

  // check if 1 explicit TC is found via request ID
  if (tcToDeleteCount == 0 || tcToDeleteCount > 1) {
    FSFW_LOGW("Service11TelecommandScheduling::doDeleteActivity: No or more than 1 TC found. "
                    "Cannot explicitly delete TC\n");
    return RETURN_FAILED;
  }

  // delete packet from store
  if (tcStore->deleteData(tcToDelete->second.storeAddr) != RETURN_OK) {
    FSFW_LOGET("Service11TelecommandScheduling::doDeleteActivity: Could not delete TC from Store\n");
    return RETURN_FAILED;
  }

  telecommandMap.erase(tcToDelete);
  if (debugMode) {
    FSFW_LOGI("PUS11::doDeleteActivity: Deleted TC from map\n");
  }

  return RETURN_OK;
}

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::doFilterDeleteActivity(
    const uint8_t *data, size_t size) {
  TcMapIter itBegin;
  TcMapIter itEnd;

  ReturnValue_t result = getMapFilterFromData(data, size, itBegin, itEnd);
  // get the filter window as map range via dedicated method
  if (result != RETURN_OK) {
    return result;
  }

  int deletedTCs = 0;
  for (TcMapIter it = itBegin; it != itEnd; it++) {
    // delete packet from store
    if (tcStore->deleteData(it->second.storeAddr) != RETURN_OK) {
      FSFW_LOGET("Service11TelecommandScheduling::doFilterDeleteActivity: Could not delete TC "
                    "from Store\n");
      continue;
    }
    deletedTCs++;
  }

  // NOTE: Spec says this function erases all elements including itBegin but not itEnd,
  // see here: https://www.cplusplus.com/reference/map/multimap/erase/
  // Therefore we need to increase itEnd by 1. (Note that end() returns the "past-the-end" iterator)
  if (itEnd != telecommandMap.end()) {
    itEnd++;
  }

  telecommandMap.erase(itBegin, itEnd);

  if (debugMode) {
    FSFW_LOGI("PUS11::doFilterDeleteActivity: Deleted {} TCs\n", deletedTCs);
  }
  return RETURN_OK;
}

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::doTimeshiftActivity(
    const uint8_t *data, size_t size) {
  // Get relative time
  uint32_t relativeTime = 0;
  ReturnValue_t result = SerializeAdapter::deSerialize(&relativeTime, &data, &size, DEF_END);
  if (result != RETURN_OK) {
    return result;
  }
  if (relativeTime == 0) {
    return INVALID_RELATIVE_TIME;
  }
  // TODO further check sanity of the relative time?

  // Get request ID
  uint64_t requestId;
  result = getRequestIdFromData(data, size, requestId);
  if (result != RETURN_OK) {
    return result;
  }

  if (debugMode) {
    FSFW_LOGI("PUS11::doTimeshiftActivity: requestId: {}\n", requestId);
  }

  // NOTE: Despite having C++17 ETL multimap has no member function extract :(

  TcMapIter tcToTimeshiftIt;
  int tcToTimeshiftCount = 0;

  for (auto it = telecommandMap.begin(); it != telecommandMap.end(); it++) {
    if (it->second.requestId == requestId) {
      tcToTimeshiftIt = it;
      tcToTimeshiftCount++;
    }
  }

  if (tcToTimeshiftCount == 0 || tcToTimeshiftCount > 1) {
    FSFW_LOGW("Service11TelecommandScheduling::doTimeshiftActivity: Either 0 or more than 1 "
                    "TCs found. No explicit timeshifting "
                    "possible\n");
    return TIMESHIFTING_NOT_POSSIBLE;
  }

  // temporarily hold the item
  TelecommandStruct tempTc(tcToTimeshiftIt->second);
  uint32_t tempKey = tcToTimeshiftIt->first + relativeTime;

  // delete old entry from the mm
  telecommandMap.erase(tcToTimeshiftIt);

  // and then insert it again as new entry
  telecommandMap.insert(std::make_pair(tempKey, tempTc));

  if (debugMode) {
    FSFW_LOGI("PUS11::doTimeshiftActivity: Shifted TC\n");
    debugPrintMultimapContent();
  }

  return RETURN_OK;
}

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::doFilterTimeshiftActivity(
    const uint8_t *data, size_t size) {
  // Get relative time
  uint32_t relativeTime = 0;
  ReturnValue_t result = SerializeAdapter::deSerialize(&relativeTime, &data, &size, DEF_END);
  if (result != RETURN_OK) {
    return result;
  }
  if (relativeTime == 0) {
    return INVALID_RELATIVE_TIME;
  }

  // Do time window
  TcMapIter itBegin;
  TcMapIter itEnd;
  result = getMapFilterFromData(data, size, itBegin, itEnd);
  if (result != RETURN_OK) {
    return result;
  }

  int shiftedItemsCount = 0;
  for (auto it = itBegin; it != itEnd;) {
    // temporarily hold the item
    TelecommandStruct tempTc(it->second);
    uint32_t tempKey = it->first + relativeTime;

    // delete the old entry from the mm
    telecommandMap.erase(it++);

    // and then insert it again as new entry
    telecommandMap.insert(std::make_pair(tempKey, tempTc));
    shiftedItemsCount++;
  }

  if (debugMode) {
    FSFW_LOGI("PUS11::doFilterTimeshiftActivity: shiftedItemsCount: {}\n", shiftedItemsCount);
    debugPrintMultimapContent();
  }

  if (shiftedItemsCount > 0) {
    return RETURN_OK;
  }
  return RETURN_FAILED;
}

template <size_t MAX_NUM_TCS>
inline uint64_t Service11TelecommandScheduling<MAX_NUM_TCS>::getRequestIdFromDataTC(
    const uint8_t *data) const {
  TcPacketPus mask(data);

  uint32_t sourceId = mask.getSourceId();
  uint16_t apid = mask.getAPID();
  uint16_t sequenceCount = mask.getPacketSequenceCount();

  return buildRequestId(sourceId, apid, sequenceCount);
}

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::getRequestIdFromData(
    const uint8_t *&data, size_t &dataSize, uint64_t &requestId) {
  uint32_t srcId = 0;
  uint16_t apid = 0;
  uint16_t ssc = 0;

  ReturnValue_t result = SerializeAdapter::deSerialize(&srcId, &data, &dataSize, DEF_END);
  if (result != RETURN_OK) {
    return result;
  }
  result = SerializeAdapter::deSerialize(&apid, &data, &dataSize, DEF_END);
  if (result != RETURN_OK) {
    return result;
  }
  result = SerializeAdapter::deSerialize(&ssc, &data, &dataSize, DEF_END);
  if (result != RETURN_OK) {
    return result;
  }
  requestId = buildRequestId(srcId, apid, ssc);

  return RETURN_OK;
}

template <size_t MAX_NUM_TCS>
inline uint64_t Service11TelecommandScheduling<MAX_NUM_TCS>::buildRequestId(uint32_t sourceId,
                                                                            uint16_t apid,
                                                                            uint16_t ssc) const {
  auto sourceId64 = static_cast<uint64_t>(sourceId);
  auto apid64 = static_cast<uint64_t>(apid);
  auto ssc64 = static_cast<uint64_t>(ssc);

  return (sourceId64 << 32) | (apid64 << 16) | ssc64;
}

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::getMapFilterFromData(
    const uint8_t *&data, size_t &dataSize, TcMapIter &itBegin, TcMapIter &itEnd) {
  // get filter type first
  uint32_t typeRaw = 0;
  ReturnValue_t result = SerializeAdapter::deSerialize(&typeRaw, &data, &dataSize, DEF_END);
  if (result != RETURN_OK) {
    return result;
  }

  if (typeRaw > 3) {
    return INVALID_TYPE_TIME_WINDOW;
  }
  auto type = static_cast<TypeOfTimeWindow>(typeRaw);

  // we now have the type of delete activity - so now we set the range to delete,
  // according to the type of time window.
  // NOTE: Blocks are used in this switch-case statement so that timestamps can be
  // cleanly redefined for each case.
  switch (type) {
    case TypeOfTimeWindow::SELECT_ALL: {
      itBegin = telecommandMap.begin();
      itEnd = telecommandMap.end();
      break;
    }

    case TypeOfTimeWindow::FROM_TIMETAG: {
      uint32_t fromTimestamp = 0;
      result = SerializeAdapter::deSerialize(&fromTimestamp, &data, &dataSize, DEF_END);
      if (result != RETURN_OK) {
        return result;
      }

      itBegin = telecommandMap.begin();
      while (itBegin->first < fromTimestamp && itBegin != telecommandMap.end()) {
        itBegin++;
      }

      itEnd = telecommandMap.end();
      break;
    }

    case TypeOfTimeWindow::TO_TIMETAG: {
      uint32_t toTimestamp;
      result = SerializeAdapter::deSerialize(&toTimestamp, &data, &dataSize, DEF_END);
      if (result != RETURN_OK) {
        return result;
      }
      itBegin = telecommandMap.begin();
      itEnd = telecommandMap.begin();
      while (itEnd->first <= toTimestamp && itEnd != telecommandMap.end()) {
        itEnd++;
      }
      break;
    }

    case TypeOfTimeWindow::FROM_TIMETAG_TO_TIMETAG: {
      uint32_t fromTimestamp;
      uint32_t toTimestamp;

      result = SerializeAdapter::deSerialize(&fromTimestamp, &data, &dataSize,
                                             SerializeIF::Endianness::BIG);
      if (result != RETURN_OK) {
        return result;
      }
      result = SerializeAdapter::deSerialize(&toTimestamp, &data, &dataSize,
                                             SerializeIF::Endianness::BIG);
      if (result != RETURN_OK) {
        return result;
      }
      itBegin = telecommandMap.begin();
      itEnd = telecommandMap.begin();

      while (itBegin->first < fromTimestamp && itBegin != telecommandMap.end()) {
        itBegin++;
      }
      while (itEnd->first <= toTimestamp && itEnd != telecommandMap.end()) {
        itEnd++;
      }
      break;
    }

    default:
      return RETURN_FAILED;
  }

  // additional security check, this should never be true
  if (itBegin->first > itEnd->first) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
#else
    sif::printError("11::GetMapFilterFromData: itBegin > itEnd\n");
#endif
    return RETURN_FAILED;
  }

  // the map range should now be set according to the sent filter.
  return RETURN_OK;
}

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::handleInvalidData(
    const char *ctx) {
#if FSFW_VERBOSE_LEVEL >= 1
  FSFW_LOGW("Service11TelecommandScheduling::{}: Invalid buffer\n", ctx);
#endif
  return RETURN_FAILED;
}

template <size_t MAX_NUM_TCS>
inline void Service11TelecommandScheduling<MAX_NUM_TCS>::debugPrintMultimapContent() const {
#if FSFW_DISABLE_PRINTOUT == 0
  FSFW_LOGD("Service11TelecommandScheduling::debugPrintMultimapContent: Multimap Content\n");
  for (const auto &dit : telecommandMap) {
    FSFW_LOGD("[{}]: Request ID: {}  | Store Address: {}\n", dit.first, dit.second.requestId,
                    dit.second.storeAddr.raw);
  }
#endif
}

template <size_t MAX_NUM_TCS>
inline void Service11TelecommandScheduling<MAX_NUM_TCS>::enableExpiredTcDeletion() {
  deleteExpiredTcWhenDisabled = true;
}

template <size_t MAX_NUM_TCS>
inline void Service11TelecommandScheduling<MAX_NUM_TCS>::disableExpiredTcDeletion() {
  deleteExpiredTcWhenDisabled = false;
}

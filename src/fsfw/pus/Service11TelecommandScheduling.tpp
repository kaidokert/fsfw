#pragma once

#include <cstddef>

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serialize/SerializeAdapter.h"
#include "fsfw/serviceinterface.h"
#include "fsfw/tmtcservices/AcceptsTelecommandsIF.h"

static constexpr auto DEF_END = SerializeIF::Endianness::BIG;

template <size_t MAX_NUM_TCS>
inline Service11TelecommandScheduling<MAX_NUM_TCS>::Service11TelecommandScheduling(
    PsbParams params, AcceptsTelecommandsIF *tcRecipient, uint16_t releaseTimeMarginSeconds,
    bool debugMode)
    : PusServiceBase(params),
      RELEASE_TIME_MARGIN_SECONDS(releaseTimeMarginSeconds),
      debugMode(debugMode),
      tcRecipient(tcRecipient) {
  params.name = "PUS 11 TC Scheduling";
}

template <size_t MAX_NUM_TCS>
inline Service11TelecommandScheduling<MAX_NUM_TCS>::~Service11TelecommandScheduling() = default;

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::handleRequest(
    uint8_t subservice) {
  if (debugMode) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "PUS11::handleRequest: Handling request " << static_cast<int>(subservice);
#else
    sif::printInfo("PUS11::handleRequest: Handling request %d\n", subservice);
#endif
  }
  // Get de-serialized Timestamp
  const uint8_t *data = currentPacket.getUserData();
  size_t size = currentPacket.getUserDataLen();
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
  return returnvalue::OK;
}

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::performService() {
  if (not schedulingEnabled) {
    return returnvalue::OK;
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
        auto sendRet = psbParams.reqQueue->sendMessage(recipientMsgQueueId, &releaseMsg, false);

        if (sendRet != returnvalue::OK) {
          return sendRet;
        }
        if (debugMode) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
          sif::info << "Released TC & erased it from TC map" << std::endl;
#else
          sif::printInfo("Released TC & erased it from TC map\n");
#endif
        }
        telecommandMap.erase(it++);
      } else if (deleteExpiredTcWhenDisabled) {
        telecommandMap.erase(it++);
      }
      continue;
    }
    it++;
  }

  return returnvalue::OK;
}

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::initialize() {
  ReturnValue_t res = PusServiceBase::initialize();
  if (res != returnvalue::OK) {
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
    if (result != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      // This should not happen
      sif::warning << "Service11TelecommandScheduling::handleRequestDeleting: Deletion failed"
                   << std::endl;
#else
      sif::printWarning("Service11TelecommandScheduling::handleRequestDeleting: Deletion failed\n");
#endif
      triggerEvent(TC_DELETION_FAILED, (it->second.requestId >> 32) & 0xffffffff,
                   it->second.requestId & 0xffffffff);
    }
  }
  telecommandMap.clear();
  return returnvalue::OK;
}

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::doInsertActivity(
    const uint8_t *data, size_t size) {
  uint32_t timestamp = 0;
  ReturnValue_t result = SerializeAdapter::deSerialize(&timestamp, &data, &size, DEF_END);
  if (result != returnvalue::OK) {
    return result;
  }

  // Insert possible if sched. time is above margin
  // (See requirement for Time margin)
  timeval tNow = {};
  Clock::getClock_timeval(&tNow);
  if (timestamp - tNow.tv_sec <= RELEASE_TIME_MARGIN_SECONDS) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "Service11TelecommandScheduling::doInsertActivity: Release time too close to "
                    "current time"
                 << std::endl;
#else
    sif::printWarning(
        "Service11TelecommandScheduling::doInsertActivity: Release time too close to current "
        "time\n");
#endif
    return returnvalue::FAILED;
  }

  // store currentPacket and receive the store address
  store_address_t addr{};
  if (tcStore->addData(&addr, data, size) != returnvalue::OK ||
      addr.raw == store_address_t::INVALID_RAW) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "Service11TelecommandScheduling::doInsertActivity: Adding data to TC Store failed"
               << std::endl;
#else
    sif::printError(
        "Service11TelecommandScheduling::doInsertActivity: Adding data to TC Store failed\n");
#endif
    return returnvalue::FAILED;
  }

  // insert into multimap with new store address
  TelecommandStruct tc;
  tc.seconds = timestamp;
  tc.storeAddr = addr;
  tc.requestId = getRequestIdFromTc();  // TODO: Missing sanity check of the returned request id

  auto it = telecommandMap.insert(std::pair<uint32_t, TelecommandStruct>(timestamp, tc));
  if (it == telecommandMap.end()) {
    return returnvalue::FAILED;
  }

  if (debugMode) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "PUS11::doInsertActivity: Inserted into Multimap:" << std::endl;
#else
    sif::printInfo("PUS11::doInsertActivity: Inserted into Multimap:\n");
#endif
    debugPrintMultimapContent();
  }
  return returnvalue::OK;
}

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::doDeleteActivity(
    const uint8_t *data, size_t size) {
  // Get request ID
  uint64_t requestId;
  ReturnValue_t result = getRequestIdFromData(data, size, requestId);
  if (result != returnvalue::OK) {
    return result;
  }

  // DEBUG
  if (debugMode) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "PUS11::doDeleteActivity: requestId: " << requestId << std::endl;
#else
    sif::printInfo("PUS11::doDeleteActivity: requestId: %d\n", requestId);
#endif
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
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "Service11TelecommandScheduling::doDeleteActivity: No or more than 1 TC found. "
                    "Cannot explicitly delete TC"
                 << std::endl;
#else
    sif::printWarning(
        "Service11TelecommandScheduling::doDeleteActivity: No or more than 1 TC found. "
        "Cannot explicitly delete TC");
#endif
    return returnvalue::FAILED;
  }

  // delete packet from store
  if (tcStore->deleteData(tcToDelete->second.storeAddr) != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "Service11TelecommandScheduling::doDeleteActivity: Could not delete TC from Store"
               << std::endl;
#else
    sif::printError(
        "Service11TelecommandScheduling::doDeleteActivity: Could not delete TC from Store\n");
#endif
    return returnvalue::FAILED;
  }

  telecommandMap.erase(tcToDelete);
  if (debugMode) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "PUS11::doDeleteActivity: Deleted TC from map" << std::endl;
#else
    sif::printInfo("PUS11::doDeleteActivity: Deleted TC from map\n");
#endif
  }

  return returnvalue::OK;
}

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::doFilterDeleteActivity(
    const uint8_t *data, size_t size) {
  TcMapIter itBegin;
  TcMapIter itEnd;

  ReturnValue_t result = getMapFilterFromData(data, size, itBegin, itEnd);
  // get the filter window as map range via dedicated method
  if (result != returnvalue::OK) {
    return result;
  }

  int deletedTCs = 0;
  for (TcMapIter it = itBegin; it != itEnd; it++) {
    // delete packet from store
    if (tcStore->deleteData(it->second.storeAddr) != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "Service11TelecommandScheduling::doFilterDeleteActivity: Could not delete TC "
                    "from Store"
                 << std::endl;
#else
      sif::printError(
          "Service11TelecommandScheduling::doFilterDeleteActivity: Could not delete TC from "
          "Store\n");
#endif
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
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "PUS11::doFilterDeleteActivity: Deleted " << deletedTCs << " TCs" << std::endl;
#else
    sif::printInfo("PUS11::doFilterDeleteActivity: Deleted %d TCs\n", deletedTCs);
#endif
  }
  return returnvalue::OK;
}

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::doTimeshiftActivity(
    const uint8_t *data, size_t size) {
  // Get relative time
  uint32_t relativeTime = 0;
  ReturnValue_t result = SerializeAdapter::deSerialize(&relativeTime, &data, &size, DEF_END);
  if (result != returnvalue::OK) {
    return result;
  }
  if (relativeTime == 0) {
    return INVALID_RELATIVE_TIME;
  }
  // TODO further check sanity of the relative time?

  // Get request ID
  uint64_t requestId;
  result = getRequestIdFromData(data, size, requestId);
  if (result != returnvalue::OK) {
    return result;
  }

  if (debugMode) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "PUS11::doTimeshiftActivity: requestId: " << requestId << std::endl;
#else
    sif::printInfo("PUS11::doTimeshiftActivity: requestId: %d\n", requestId);
#endif
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
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "Service11TelecommandScheduling::doTimeshiftActivity: Either 0 or more than 1 "
                    "TCs found. No explicit timeshifting "
                    "possible"
                 << std::endl;

#else
    sif::printWarning(
        "Service11TelecommandScheduling::doTimeshiftActivity: Either 0 or more than 1 TCs found. "
        "No explicit timeshifting possible\n");
#endif
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
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "PUS11::doTimeshiftActivity: Shifted TC" << std::endl;
#else
    sif::printDebug("PUS11::doTimeshiftActivity: Shifted TC\n");
#endif
    debugPrintMultimapContent();
  }

  return returnvalue::OK;
}

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::doFilterTimeshiftActivity(
    const uint8_t *data, size_t size) {
  // Get relative time
  uint32_t relativeTime = 0;
  ReturnValue_t result = SerializeAdapter::deSerialize(&relativeTime, &data, &size, DEF_END);
  if (result != returnvalue::OK) {
    return result;
  }
  if (relativeTime == 0) {
    return INVALID_RELATIVE_TIME;
  }

  // Do time window
  TcMapIter itBegin;
  TcMapIter itEnd;
  result = getMapFilterFromData(data, size, itBegin, itEnd);
  if (result != returnvalue::OK) {
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
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "PUS11::doFilterTimeshiftActivity: shiftedItemsCount: " << shiftedItemsCount
              << std::endl;
#else
    sif::printInfo("PUS11::doFilterTimeshiftActivity: shiftedItemsCount: %d\n", shiftedItemsCount);
#endif
    debugPrintMultimapContent();
  }

  if (shiftedItemsCount > 0) {
    return returnvalue::OK;
  }
  return returnvalue::FAILED;
}

template <size_t MAX_NUM_TCS>
inline uint64_t Service11TelecommandScheduling<MAX_NUM_TCS>::getRequestIdFromTc() const {
  uint32_t sourceId = currentPacket.getSourceId();
  uint16_t apid = currentPacket.getApid();
  uint16_t sequenceCount = currentPacket.getSequenceCount();

  return buildRequestId(sourceId, apid, sequenceCount);
}

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::getRequestIdFromData(
    const uint8_t *&data, size_t &dataSize, uint64_t &requestId) {
  uint32_t srcId = 0;
  uint16_t apid = 0;
  uint16_t ssc = 0;

  ReturnValue_t result = SerializeAdapter::deSerialize(&srcId, &data, &dataSize, DEF_END);
  if (result != returnvalue::OK) {
    return result;
  }
  result = SerializeAdapter::deSerialize(&apid, &data, &dataSize, DEF_END);
  if (result != returnvalue::OK) {
    return result;
  }
  result = SerializeAdapter::deSerialize(&ssc, &data, &dataSize, DEF_END);
  if (result != returnvalue::OK) {
    return result;
  }
  requestId = buildRequestId(srcId, apid, ssc);

  return returnvalue::OK;
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
  if (result != returnvalue::OK) {
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
      if (result != returnvalue::OK) {
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
      if (result != returnvalue::OK) {
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
      if (result != returnvalue::OK) {
        return result;
      }
      result = SerializeAdapter::deSerialize(&toTimestamp, &data, &dataSize,
                                             SerializeIF::Endianness::BIG);
      if (result != returnvalue::OK) {
        return result;
      }
      if (fromTimestamp > toTimestamp) {
        return INVALID_TIME_WINDOW;
      }
      itBegin = telecommandMap.begin();

      while (itBegin->first < fromTimestamp && itBegin != telecommandMap.end()) {
        itBegin++;
      }

      // start looking for end beginning at begin
      itEnd = itBegin;
      while (itEnd->first <= toTimestamp && itEnd != telecommandMap.end()) {
        itEnd++;
      }
      break;
    }

    default:
      return returnvalue::FAILED;
  }
  // the map range should now be set according to the sent filter.
  return returnvalue::OK;
}

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::handleInvalidData(
    const char *ctx) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::warning << "Service11TelecommandScheduling:: " << ctx << ": Invalid buffer" << std::endl;
#else
  sif::printWarning("Service11TelecommandScheduling::%s: Invalid buffer\n", ctx);
#endif
#endif
  return returnvalue::FAILED;
}

template <size_t MAX_NUM_TCS>
inline void Service11TelecommandScheduling<MAX_NUM_TCS>::debugPrintMultimapContent() const {
#if FSFW_DISABLE_PRINTOUT == 0
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::debug << "Service11TelecommandScheduling::debugPrintMultimapContent: Multimap Content"
             << std::endl;
#else
  sif::printDebug("Service11TelecommandScheduling::debugPrintMultimapContent: Multimap Content\n");
#endif
  for (const auto &dit : telecommandMap) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "[" << dit.first << "]: Request ID: " << dit.second.requestId << " | "
               << "Store Address: " << dit.second.storeAddr.raw << std::endl;
#else
    sif::printDebug("[%d]: Request ID: %d  | Store Address: %d\n", dit.first, dit.second.requestId,
                    dit.second.storeAddr);
#endif
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

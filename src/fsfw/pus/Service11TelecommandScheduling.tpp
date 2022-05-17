#pragma once

#include <fsfw/objectmanager/ObjectManager.h>
#include <fsfw/serialize/SerializeAdapter.h>
#include <fsfw/tmtcservices/AcceptsTelecommandsIF.h>

#include <cstddef>

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
inline Service11TelecommandScheduling<MAX_NUM_TCS>::~Service11TelecommandScheduling() {}

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
  const uint8_t *data = currentPacket.getApplicationData();
  size_t size = currentPacket.getApplicationDataSize();
  if (data == nullptr) {
    return handleInvalidData("handleRequest");
  }
  switch (subservice) {
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
      break;
  }

  return HasReturnvaluesIF::RETURN_FAILED;
}

template <size_t MAX_NUM_TCS>
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::performService() {
  // DEBUG
  // DebugPrintMultimapContent();

  // get current time as UNIX timestamp
  timeval tNow = {};
  Clock::getClock_timeval(&tNow);

  // NOTE: The iterator is increased in the loop here. Increasing the iterator as for-loop arg
  // does not work in this case as we are deleting the current element here.
  for (auto it = telecommandMap.begin(); it != telecommandMap.end();) {
    if (it->first <= tNow.tv_sec) {
      // release tc
      TmTcMessage releaseMsg(it->second.storeAddr);
      auto sendRet = this->requestQueue->sendMessage(recipientMsgQueueId, &releaseMsg, false);

      if (sendRet != HasReturnvaluesIF::RETURN_OK) {
        return sendRet;
      }

      telecommandMap.erase(it++);

      if (debugMode) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::info << "Released TC & erased it from TC map" << std::endl;
#else
        sif::printInfo("Released TC & erased it from TC map\n");
#endif
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
inline ReturnValue_t Service11TelecommandScheduling<MAX_NUM_TCS>::doInsertActivity(
    const uint8_t *data, size_t size) {
  uint32_t timestamp = 0;
  const uint8_t *initData = data;
  size_t initSz = size;
  ReturnValue_t result = SerializeAdapter::deSerialize(&timestamp, &data, &size, DEF_END);
  if (result != RETURN_OK) {
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
    return RETURN_FAILED;
  }

  // store currentPacket and receive the store address
  store_address_t addr{};
  if (tcStore->addData(&addr, initData, initSz) != RETURN_OK ||
      addr.raw == storeId::INVALID_STORE_ADDRESS) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "Service11TelecommandScheduling::doInsertActivity: Adding data to TC Store failed"
               << std::endl;
#else
    sif::printError(
        "Service11TelecommandScheduling::doInsertActivity: Adding data to TC Store failed\n");
#endif
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
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "PUS11::doInsertActivity: Inserted into Multimap:" << std::endl;
#else
    sif::printInfo("PUS11::doInsertActivity: Inserted into Multimap:\n");
#endif
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
    return RETURN_FAILED;
  }

  // delete packet from store
  if (tcStore->deleteData(tcToDelete->second.storeAddr) != RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "Service11TelecommandScheduling::doDeleteActivity: Could not delete TC from Store"
               << std::endl;
#else
    sif::printError(
        "Service11TelecommandScheduling::doDeleteActivity: Could not delete TC from Store\n");
#endif
    return RETURN_FAILED;
  }

  telecommandMap.erase(tcToDelete);
  if (debugMode) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "PUS11::doDeleteActivity: Deleted TC from map" << std::endl;
#else
    sif::printInfo("PUS11::doDeleteActivity: Deleted TC from map\n");
#endif
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
    continue;
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
  uint64_t sourceId64 = static_cast<uint64_t>(sourceId);
  uint64_t apid64 = static_cast<uint64_t>(apid);
  uint64_t ssc64 = static_cast<uint64_t>(ssc);

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
  TypeOfTimeWindow type = static_cast<TypeOfTimeWindow>(typeRaw);

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
    sif::printError("11::GetMapFilterFromData: itBegin > itEnd\n");
    return RETURN_FAILED;
  }

  // the map range should now be set according to the sent filter.
  return RETURN_OK;
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
  return RETURN_FAILED;
}

template <size_t MAX_NUM_TCS>
inline void Service11TelecommandScheduling<MAX_NUM_TCS>::debugPrintMultimapContent(void) const {
#if FSFW_DISABLE_PRINTOUT == 0
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::debug << "Service11TelecommandScheduling::debugPrintMultimapContent: Multimap Content"
             << std::endl;
  sif::debug << "[" << dit->first << "]: Request ID: " << dit->second.requestId << " | "
             << "Store Address: " << dit->second.storeAddr << std::endl;
#else
  sif::printDebug("Service11TelecommandScheduling::debugPrintMultimapContent: Multimap Content\n");
  for (auto dit = telecommandMap.begin(); dit != telecommandMap.end(); ++dit) {
    sif::printDebug("[%d]: Request ID: %d  | Store Address: %d\n", dit->first,
                    dit->second.requestId, dit->second.storeAddr);
  }
#endif
#endif
}

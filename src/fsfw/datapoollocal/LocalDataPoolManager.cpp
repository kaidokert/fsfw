#include "fsfw/datapoollocal/LocalDataPoolManager.h"

#include <cmath>

#include "fsfw/datapoollocal.h"
#include "fsfw/housekeeping/AcceptsHkPacketsIF.h"
#include "fsfw/housekeeping/HousekeepingSetPacket.h"
#include "fsfw/housekeeping/HousekeepingSnapshot.h"
#include "fsfw/ipc/MutexFactory.h"
#include "fsfw/ipc/MutexGuard.h"
#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/timemanager/CCSDSTime.h"
#include "internal/HasLocalDpIFManagerAttorney.h"
#include "internal/LocalPoolDataSetAttorney.h"

// TODO: Get rid of this. This should be a constructor argument, not something hardcoded in any way
object_id_t LocalDataPoolManager::defaultHkDestination = objects::PUS_SERVICE_3_HOUSEKEEPING;

LocalDataPoolManager::LocalDataPoolManager(HasLocalDataPoolIF* owner, MessageQueueIF* queueToUse,
                                           bool appendValidityBuffer)
    : appendValidityBuffer(appendValidityBuffer) {
  if (owner == nullptr) {
    printWarningOrError(sif::OutputTypes::OUT_WARNING, "LocalDataPoolManager", returnvalue::FAILED,
                        "Invalid supplied owner");
    return;
  }
  this->owner = owner;
  mutex = MutexFactory::instance()->createMutex();
  if (mutex == nullptr) {
    printWarningOrError(sif::OutputTypes::OUT_ERROR, "LocalDataPoolManager", returnvalue::FAILED,
                        "Could not create mutex");
  }

  hkQueue = queueToUse;
}

LocalDataPoolManager::~LocalDataPoolManager() {
  if (mutex != nullptr) {
    MutexFactory::instance()->deleteMutex(mutex);
  }
}

ReturnValue_t LocalDataPoolManager::initialize(MessageQueueIF* queueToUse) {
  if (queueToUse == nullptr) {
    /* Error, all destinations invalid */
    printWarningOrError(sif::OutputTypes::OUT_ERROR, "initialize", QUEUE_OR_DESTINATION_INVALID);
  }
  hkQueue = queueToUse;

  ipcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
  if (ipcStore == nullptr) {
    /* Error, all destinations invalid */
    printWarningOrError(sif::OutputTypes::OUT_ERROR, "initialize", returnvalue::FAILED,
                        "Could not set IPC store.");
    return returnvalue::FAILED;
  }

  if (defaultHkDestination != objects::NO_OBJECT) {
    auto* hkPacketReceiver =
        ObjectManager::instance()->get<AcceptsHkPacketsIF>(defaultHkDestination);
    if (hkPacketReceiver != nullptr) {
      hkDestinationId = hkPacketReceiver->getHkQueue();
    } else {
      printWarningOrError(sif::OutputTypes::OUT_ERROR, "initialize", QUEUE_OR_DESTINATION_INVALID);
      return QUEUE_OR_DESTINATION_INVALID;
    }
  }

  return returnvalue::OK;
}

ReturnValue_t LocalDataPoolManager::initializeAfterTaskCreation(uint8_t nonDiagInvlFactor) {
  setNonDiagnosticIntervalFactor(nonDiagInvlFactor);
  return initializeHousekeepingPoolEntriesOnce();
}

ReturnValue_t LocalDataPoolManager::initializeHousekeepingPoolEntriesOnce() {
  if (not mapInitialized) {
    ReturnValue_t result = owner->initializeLocalDataPool(localPoolMap, *this);
    if (result == returnvalue::OK) {
      mapInitialized = true;
    }
    return result;
  }

  printWarningOrError(sif::OutputTypes::OUT_WARNING, "initializeHousekeepingPoolEntriesOnce",
                      returnvalue::FAILED, "The map should only be initialized once");
  return returnvalue::OK;
}

ReturnValue_t LocalDataPoolManager::performHkOperation() {
  ReturnValue_t status = returnvalue::OK;
  for (auto& receiver : hkReceivers) {
    switch (receiver.reportingType) {
      case (ReportingType::PERIODIC): {
        if (receiver.dataType == DataType::LOCAL_POOL_VARIABLE) {
          /* Periodic packets shall only be generated from datasets */
          continue;
        }
        performPeriodicHkGeneration(receiver);
        break;
      }
      case (ReportingType::UPDATE_HK): {
        handleHkUpdate(receiver, status);
        break;
      }
      case (ReportingType::UPDATE_NOTIFICATION): {
        handleNotificationUpdate(receiver, status);
        break;
      }
      case (ReportingType::UPDATE_SNAPSHOT): {
        handleNotificationSnapshot(receiver, status);
        break;
      }
      default:
        // This should never happen.
        return returnvalue::FAILED;
    }
  }
  resetHkUpdateResetHelper();
  return status;
}

ReturnValue_t LocalDataPoolManager::handleHkUpdate(HkReceiver& receiver, ReturnValue_t& status) {
  if (receiver.dataType == DataType::LOCAL_POOL_VARIABLE) {
    /* Update packets shall only be generated from datasets. */
    return returnvalue::FAILED;
  }
  LocalPoolDataSetBase* dataSet =
      HasLocalDpIFManagerAttorney::getDataSetHandle(owner, receiver.dataId.sid);
  if (dataSet == nullptr) {
    return DATASET_NOT_FOUND;
  }
  if (dataSet->hasChanged()) {
    /* Prepare and send update notification */
    ReturnValue_t result = generateHousekeepingPacket(receiver.dataId.sid, dataSet, true);
    if (result != returnvalue::OK) {
      status = result;
    }
  }
  handleChangeResetLogic(receiver.dataType, receiver.dataId, dataSet);
  return returnvalue::OK;
}

ReturnValue_t LocalDataPoolManager::handleNotificationUpdate(HkReceiver& receiver,
                                                             ReturnValue_t& status) {
  MarkChangedIF* toReset = nullptr;
  if (receiver.dataType == DataType::LOCAL_POOL_VARIABLE) {
    LocalPoolObjectBase* poolObj =
        HasLocalDpIFManagerAttorney::getPoolObjectHandle(owner, receiver.dataId.localPoolId);
    if (poolObj == nullptr) {
      printWarningOrError(sif::OutputTypes::OUT_WARNING, "handleNotificationUpdate",
                          POOLOBJECT_NOT_FOUND);
      return POOLOBJECT_NOT_FOUND;
    }
    if (poolObj->hasChanged()) {
      /* Prepare and send update notification. */
      CommandMessage notification;
      HousekeepingMessage::setUpdateNotificationVariableCommand(
          &notification, gp_id_t(owner->getObjectId(), receiver.dataId.localPoolId));
      ReturnValue_t result = hkQueue->sendMessage(receiver.destinationQueue, &notification);
      if (result != returnvalue::OK) {
        status = result;
      }
      toReset = poolObj;
    }

  } else {
    LocalPoolDataSetBase* dataSet =
        HasLocalDpIFManagerAttorney::getDataSetHandle(owner, receiver.dataId.sid);
    if (dataSet == nullptr) {
      printWarningOrError(sif::OutputTypes::OUT_WARNING, "handleNotificationUpdate",
                          DATASET_NOT_FOUND);
      return DATASET_NOT_FOUND;
    }
    if (dataSet->hasChanged()) {
      /* Prepare and send update notification */
      CommandMessage notification;
      HousekeepingMessage::setUpdateNotificationSetCommand(&notification, receiver.dataId.sid);
      ReturnValue_t result = hkQueue->sendMessage(receiver.destinationQueue, &notification);
      if (result != returnvalue::OK) {
        status = result;
      }
      toReset = dataSet;
    }
  }
  if (toReset != nullptr) {
    handleChangeResetLogic(receiver.dataType, receiver.dataId, toReset);
  }
  return returnvalue::OK;
}

ReturnValue_t LocalDataPoolManager::handleNotificationSnapshot(HkReceiver& receiver,
                                                               ReturnValue_t& status) {
  MarkChangedIF* toReset = nullptr;
  /* Check whether data has changed and send messages in case it has */
  if (receiver.dataType == DataType::LOCAL_POOL_VARIABLE) {
    LocalPoolObjectBase* poolObj =
        HasLocalDpIFManagerAttorney::getPoolObjectHandle(owner, receiver.dataId.localPoolId);
    if (poolObj == nullptr) {
      printWarningOrError(sif::OutputTypes::OUT_WARNING, "handleNotificationSnapshot",
                          POOLOBJECT_NOT_FOUND);
      return POOLOBJECT_NOT_FOUND;
    }

    if (not poolObj->hasChanged()) {
      return returnvalue::OK;
    }

    /* Prepare and send update snapshot */
    timeval now{};
    Clock::getClock_timeval(&now);
    CCSDSTime::CDS_short cds{};
    CCSDSTime::convertToCcsds(&cds, &now);
    HousekeepingSnapshot updatePacket(
        reinterpret_cast<uint8_t*>(&cds), sizeof(cds),
        HasLocalDpIFManagerAttorney::getPoolObjectHandle(owner, receiver.dataId.localPoolId));

    store_address_t storeId;
    ReturnValue_t result = addUpdateToStore(updatePacket, storeId);
    if (result != returnvalue::OK) {
      return result;
    }

    CommandMessage notification;
    HousekeepingMessage::setUpdateSnapshotVariableCommand(
        &notification, gp_id_t(owner->getObjectId(), receiver.dataId.localPoolId), storeId);
    result = hkQueue->sendMessage(receiver.destinationQueue, &notification);
    if (result != returnvalue::OK) {
      status = result;
    }
    toReset = poolObj;
  } else {
    LocalPoolDataSetBase* dataSet =
        HasLocalDpIFManagerAttorney::getDataSetHandle(owner, receiver.dataId.sid);
    if (dataSet == nullptr) {
      printWarningOrError(sif::OutputTypes::OUT_WARNING, "handleNotificationSnapshot",
                          DATASET_NOT_FOUND);
      return DATASET_NOT_FOUND;
    }

    if (not dataSet->hasChanged()) {
      return returnvalue::OK;
    }

    /* Prepare and send update snapshot */
    timeval now{};
    Clock::getClock_timeval(&now);
    CCSDSTime::CDS_short cds{};
    CCSDSTime::convertToCcsds(&cds, &now);
    HousekeepingSnapshot updatePacket(
        reinterpret_cast<uint8_t*>(&cds), sizeof(cds),
        HasLocalDpIFManagerAttorney::getDataSetHandle(owner, receiver.dataId.sid));

    store_address_t storeId;
    ReturnValue_t result = addUpdateToStore(updatePacket, storeId);
    if (result != returnvalue::OK) {
      return result;
    }

    CommandMessage notification;
    HousekeepingMessage::setUpdateSnapshotSetCommand(&notification, receiver.dataId.sid, storeId);
    result = hkQueue->sendMessage(receiver.destinationQueue, &notification);
    if (result != returnvalue::OK) {
      status = result;
    }
    toReset = dataSet;
  }
  if (toReset != nullptr) {
    handleChangeResetLogic(receiver.dataType, receiver.dataId, toReset);
  }
  return returnvalue::OK;
}

ReturnValue_t LocalDataPoolManager::addUpdateToStore(HousekeepingSnapshot& updatePacket,
                                                     store_address_t& storeId) {
  size_t updatePacketSize = updatePacket.getSerializedSize();
  uint8_t* storePtr = nullptr;
  ReturnValue_t result =
      ipcStore->getFreeElement(&storeId, updatePacket.getSerializedSize(), &storePtr);
  if (result != returnvalue::OK) {
    return result;
  }
  size_t serializedSize = 0;
  result = updatePacket.serialize(&storePtr, &serializedSize, updatePacketSize,
                                  SerializeIF::Endianness::MACHINE);
  return result;
  ;
}

void LocalDataPoolManager::handleChangeResetLogic(DataType type, DataId dataId,
                                                  MarkChangedIF* toReset) {
  for (auto& changeInfo : hkUpdateResetList) {
    if (changeInfo.dataType != type) {
      continue;
    }
    if ((changeInfo.dataType == DataType::DATA_SET) and (changeInfo.dataId.sid != dataId.sid)) {
      continue;
    }
    if ((changeInfo.dataType == DataType::LOCAL_POOL_VARIABLE) and
        (changeInfo.dataId.localPoolId != dataId.localPoolId)) {
      continue;
    }

    /* Only one update recipient, we can reset changes status immediately */
    if (changeInfo.updateCounter <= 1) {
      toReset->setChanged(false);
    }
    /* All recipients have been notified, reset the changed flag */
    else if (changeInfo.currentUpdateCounter <= 1) {
      toReset->setChanged(false);
      changeInfo.currentUpdateCounter = 0;
    }
    /* Not all recipiens have been notified yet, decrement */
    else {
      changeInfo.currentUpdateCounter--;
    }
    return;
  }
}

void LocalDataPoolManager::resetHkUpdateResetHelper() {
  for (auto& changeInfo : hkUpdateResetList) {
    changeInfo.currentUpdateCounter = changeInfo.updateCounter;
  }
}

ReturnValue_t LocalDataPoolManager::subscribeForRegularPeriodicPacket(
    subdp::RegularHkPeriodicParams params) {
  return subscribeForPeriodicPacket(params);
}

ReturnValue_t LocalDataPoolManager::subscribeForDiagPeriodicPacket(
    subdp::DiagnosticsHkPeriodicParams params) {
  return subscribeForPeriodicPacket(params);
}

ReturnValue_t LocalDataPoolManager::subscribeForPeriodicPacket(subdp::ParamsBase& params) {
  struct HkReceiver hkReceiver;
  hkReceiver.dataId.sid = params.sid;
  hkReceiver.reportingType = ReportingType::PERIODIC;
  hkReceiver.dataType = DataType::DATA_SET;
  if (params.receiver == MessageQueueIF::NO_QUEUE) {
    hkReceiver.destinationQueue = hkDestinationId;
  } else {
    hkReceiver.destinationQueue = params.receiver;
  }

  LocalPoolDataSetBase* dataSet = HasLocalDpIFManagerAttorney::getDataSetHandle(owner, params.sid);
  if (dataSet != nullptr) {
    LocalPoolDataSetAttorney::setReportingEnabled(*dataSet, params.enableReporting);
    LocalPoolDataSetAttorney::setDiagnostic(*dataSet, params.isDiagnostics());
    LocalPoolDataSetAttorney::initializePeriodicHelper(*dataSet, params.collectionInterval,
                                                       owner->getPeriodicOperationFrequency());
  }

  hkReceivers.push_back(hkReceiver);
  return returnvalue::OK;
}

ReturnValue_t LocalDataPoolManager::subscribeForRegularUpdatePacket(
    subdp::RegularHkUpdateParams params) {
  return subscribeForUpdatePacket(params);
}
ReturnValue_t LocalDataPoolManager::subscribeForDiagUpdatePacket(
    subdp::DiagnosticsHkUpdateParams params) {
  return subscribeForUpdatePacket(params);
}

ReturnValue_t LocalDataPoolManager::subscribeForUpdatePacket(subdp::ParamsBase& params) {
  struct HkReceiver hkReceiver;
  hkReceiver.dataId.sid = params.sid;
  hkReceiver.reportingType = ReportingType::UPDATE_HK;
  hkReceiver.dataType = DataType::DATA_SET;
  if (params.receiver == MessageQueueIF::NO_QUEUE) {
    hkReceiver.destinationQueue = hkDestinationId;
  } else {
    hkReceiver.destinationQueue = params.receiver;
  }

  LocalPoolDataSetBase* dataSet = HasLocalDpIFManagerAttorney::getDataSetHandle(owner, params.sid);
  if (dataSet != nullptr) {
    LocalPoolDataSetAttorney::setReportingEnabled(*dataSet, true);
    LocalPoolDataSetAttorney::setDiagnostic(*dataSet, params.isDiagnostics());
  }

  hkReceivers.push_back(hkReceiver);

  handleHkUpdateResetListInsertion(hkReceiver.dataType, hkReceiver.dataId);
  return returnvalue::OK;
}

ReturnValue_t LocalDataPoolManager::subscribeForSetUpdateMessage(const uint32_t setId,
                                                                 object_id_t destinationObject,
                                                                 MessageQueueId_t targetQueueId,
                                                                 bool generateSnapshot) {
  struct HkReceiver hkReceiver;
  hkReceiver.dataType = DataType::DATA_SET;
  hkReceiver.dataId.sid = sid_t(owner->getObjectId(), setId);
  hkReceiver.destinationQueue = targetQueueId;
  hkReceiver.objectId = destinationObject;
  if (generateSnapshot) {
    hkReceiver.reportingType = ReportingType::UPDATE_SNAPSHOT;
  } else {
    hkReceiver.reportingType = ReportingType::UPDATE_NOTIFICATION;
  }

  hkReceivers.push_back(hkReceiver);

  handleHkUpdateResetListInsertion(hkReceiver.dataType, hkReceiver.dataId);
  return returnvalue::OK;
}

ReturnValue_t LocalDataPoolManager::subscribeForVariableUpdateMessage(
    const lp_id_t localPoolId, object_id_t destinationObject, MessageQueueId_t targetQueueId,
    bool generateSnapshot) {
  struct HkReceiver hkReceiver;
  hkReceiver.dataType = DataType::LOCAL_POOL_VARIABLE;
  hkReceiver.dataId.localPoolId = localPoolId;
  hkReceiver.destinationQueue = targetQueueId;
  hkReceiver.objectId = destinationObject;
  if (generateSnapshot) {
    hkReceiver.reportingType = ReportingType::UPDATE_SNAPSHOT;
  } else {
    hkReceiver.reportingType = ReportingType::UPDATE_NOTIFICATION;
  }

  hkReceivers.push_back(hkReceiver);

  handleHkUpdateResetListInsertion(hkReceiver.dataType, hkReceiver.dataId);
  return returnvalue::OK;
}

void LocalDataPoolManager::handleHkUpdateResetListInsertion(DataType dataType, DataId dataId) {
  for (auto& updateResetStruct : hkUpdateResetList) {
    if (dataType == DataType::DATA_SET) {
      if (updateResetStruct.dataId.sid == dataId.sid) {
        updateResetStruct.updateCounter++;
        updateResetStruct.currentUpdateCounter++;
        return;
      }
    } else {
      if (updateResetStruct.dataId.localPoolId == dataId.localPoolId) {
        updateResetStruct.updateCounter++;
        updateResetStruct.currentUpdateCounter++;
        return;
      }
    }
  }
  HkUpdateResetHelper hkUpdateResetHelper;
  hkUpdateResetHelper.currentUpdateCounter = 1;
  hkUpdateResetHelper.updateCounter = 1;
  hkUpdateResetHelper.dataType = dataType;
  if (dataType == DataType::DATA_SET) {
    hkUpdateResetHelper.dataId.sid = dataId.sid;
  } else {
    hkUpdateResetHelper.dataId.localPoolId = dataId.localPoolId;
  }
  hkUpdateResetList.push_back(hkUpdateResetHelper);
}

ReturnValue_t LocalDataPoolManager::handleHousekeepingMessage(CommandMessage* message) {
  Command_t command = message->getCommand();
  sid_t sid = HousekeepingMessage::getSid(message);
  ReturnValue_t result = returnvalue::OK;
  switch (command) {
    // Houskeeping interface handling.
    case (HousekeepingMessage::ENABLE_PERIODIC_DIAGNOSTICS_GENERATION): {
      result = togglePeriodicGeneration(sid, true, true);
      break;
    }

    case (HousekeepingMessage::DISABLE_PERIODIC_DIAGNOSTICS_GENERATION): {
      result = togglePeriodicGeneration(sid, false, true);
      break;
    }

    case (HousekeepingMessage::ENABLE_PERIODIC_HK_REPORT_GENERATION): {
      result = togglePeriodicGeneration(sid, true, false);
      break;
    }

    case (HousekeepingMessage::DISABLE_PERIODIC_HK_REPORT_GENERATION): {
      result = togglePeriodicGeneration(sid, false, false);
      break;
    }

    case (HousekeepingMessage::REPORT_DIAGNOSTICS_REPORT_STRUCTURES): {
      result = generateSetStructurePacket(sid, true);
      if (result == returnvalue::OK) {
        return result;
      }
      break;
    }

    case (HousekeepingMessage::REPORT_HK_REPORT_STRUCTURES): {
      result = generateSetStructurePacket(sid, false);
      if (result == returnvalue::OK) {
        return result;
      }
      break;
    }
    case (HousekeepingMessage::MODIFY_DIAGNOSTICS_REPORT_COLLECTION_INTERVAL):
    case (HousekeepingMessage::MODIFY_PARAMETER_REPORT_COLLECTION_INTERVAL): {
      float newCollIntvl = 0;
      HousekeepingMessage::getCollectionIntervalModificationCommand(message, &newCollIntvl);
      if (command == HousekeepingMessage::MODIFY_DIAGNOSTICS_REPORT_COLLECTION_INTERVAL) {
        result = changeCollectionInterval(sid, newCollIntvl, true);
      } else {
        result = changeCollectionInterval(sid, newCollIntvl, false);
      }
      break;
    }

    case (HousekeepingMessage::GENERATE_ONE_PARAMETER_REPORT):
    case (HousekeepingMessage::GENERATE_ONE_DIAGNOSTICS_REPORT): {
      LocalPoolDataSetBase* dataSet = HasLocalDpIFManagerAttorney::getDataSetHandle(owner, sid);
      if (dataSet == nullptr) {
        printWarningOrError(sif::OutputTypes::OUT_WARNING, "handleHousekeepingMessage",
                            DATASET_NOT_FOUND);
        return DATASET_NOT_FOUND;
      }
      if (command == HousekeepingMessage::GENERATE_ONE_PARAMETER_REPORT and
          LocalPoolDataSetAttorney::isDiagnostics(*dataSet)) {
        result = WRONG_HK_PACKET_TYPE;
        break;
      } else if (command == HousekeepingMessage::GENERATE_ONE_DIAGNOSTICS_REPORT and
                 not LocalPoolDataSetAttorney::isDiagnostics(*dataSet)) {
        result = WRONG_HK_PACKET_TYPE;
        break;
      }
      return generateHousekeepingPacket(HousekeepingMessage::getSid(message), dataSet, true);
    }

    /* Notification handling */
    case (HousekeepingMessage::UPDATE_NOTIFICATION_SET): {
      owner->handleChangedDataset(sid);
      return returnvalue::OK;
    }
    case (HousekeepingMessage::UPDATE_NOTIFICATION_VARIABLE): {
      gp_id_t globPoolId = HousekeepingMessage::getUpdateNotificationVariableCommand(message);
      owner->handleChangedPoolVariable(globPoolId);
      return returnvalue::OK;
    }
    case (HousekeepingMessage::UPDATE_SNAPSHOT_SET): {
      store_address_t storeId;
      HousekeepingMessage::getUpdateSnapshotSetCommand(message, &storeId);
      bool clearMessage = true;
      owner->handleChangedDataset(sid, storeId, &clearMessage);
      if (clearMessage) {
        message->clear();
      }
      return returnvalue::OK;
    }
    case (HousekeepingMessage::UPDATE_SNAPSHOT_VARIABLE): {
      store_address_t storeId;
      gp_id_t globPoolId = HousekeepingMessage::getUpdateSnapshotVariableCommand(message, &storeId);
      bool clearMessage = true;
      owner->handleChangedPoolVariable(globPoolId, storeId, &clearMessage);
      if (clearMessage) {
        message->clear();
      }
      return returnvalue::OK;
    }

    default:
      return CommandMessageIF::UNKNOWN_COMMAND;
  }

  CommandMessage reply;
  if (result != returnvalue::OK) {
    HousekeepingMessage::setHkRequestFailureReply(&reply, sid, result);
  } else {
    HousekeepingMessage::setHkRequestSuccessReply(&reply, sid);
  }
  hkQueue->sendMessage(hkDestinationId, &reply);
  return result;
}

ReturnValue_t LocalDataPoolManager::printPoolEntry(lp_id_t localPoolId) {
  auto poolIter = localPoolMap.find(localPoolId);
  if (poolIter == localPoolMap.end()) {
    printWarningOrError(sif::OutputTypes::OUT_WARNING, "printPoolEntry",
                        localpool::POOL_ENTRY_NOT_FOUND);
    return localpool::POOL_ENTRY_NOT_FOUND;
  }
  poolIter->second->print();
  return returnvalue::OK;
}

MutexIF* LocalDataPoolManager::getMutexHandle() { return mutex; }

HasLocalDataPoolIF* LocalDataPoolManager::getOwner() { return owner; }

ReturnValue_t LocalDataPoolManager::generateHousekeepingPacket(sid_t sid,
                                                               LocalPoolDataSetBase* dataSet,
                                                               bool forDownlink,
                                                               MessageQueueId_t destination) {
  if (dataSet == nullptr) {
    /* Configuration error. */
    printWarningOrError(sif::OutputTypes::OUT_WARNING, "generateHousekeepingPacket",
                        DATASET_NOT_FOUND);
    return DATASET_NOT_FOUND;
  }

  store_address_t storeId;
  HousekeepingPacketDownlink hkPacket(sid, dataSet);
  size_t serializedSize = 0;
  ReturnValue_t result =
      serializeHkPacketIntoStore(hkPacket, storeId, forDownlink, &serializedSize);
  if (result != returnvalue::OK or serializedSize == 0) {
    return result;
  }

  /* Now we set a HK message and send it the HK packet destination. */
  CommandMessage hkMessage;
  if (LocalPoolDataSetAttorney::isDiagnostics(*dataSet)) {
    HousekeepingMessage::setHkDiagnosticsReply(&hkMessage, sid, storeId);
  } else {
    HousekeepingMessage::setHkReportReply(&hkMessage, sid, storeId);
  }

  if (hkQueue == nullptr) {
    /* Error, no queue available to send packet with. */
    printWarningOrError(sif::OutputTypes::OUT_WARNING, "generateHousekeepingPacket",
                        QUEUE_OR_DESTINATION_INVALID);
    return QUEUE_OR_DESTINATION_INVALID;
  }
  if (destination == MessageQueueIF::NO_QUEUE) {
    if (hkDestinationId == MessageQueueIF::NO_QUEUE) {
      /* Error, all destinations invalid */
      printWarningOrError(sif::OutputTypes::OUT_WARNING, "generateHousekeepingPacket",
                          QUEUE_OR_DESTINATION_INVALID);
      return QUEUE_OR_DESTINATION_INVALID;
    }
    destination = hkDestinationId;
  }

  return hkQueue->sendMessage(destination, &hkMessage);
}

ReturnValue_t LocalDataPoolManager::serializeHkPacketIntoStore(HousekeepingPacketDownlink& hkPacket,
                                                               store_address_t& storeId,
                                                               bool forDownlink,
                                                               size_t* serializedSize) {
  uint8_t* dataPtr = nullptr;
  const size_t maxSize = hkPacket.getSerializedSize();
  ReturnValue_t result = ipcStore->getFreeElement(&storeId, maxSize, &dataPtr);
  if (result != returnvalue::OK) {
    return result;
  }

  if (forDownlink) {
    return hkPacket.serialize(&dataPtr, serializedSize, maxSize, SerializeIF::Endianness::BIG);
  }
  return hkPacket.serialize(&dataPtr, serializedSize, maxSize, SerializeIF::Endianness::MACHINE);
}

void LocalDataPoolManager::setNonDiagnosticIntervalFactor(uint8_t nonDiagInvlFactor) {
  this->nonDiagnosticIntervalFactor = nonDiagInvlFactor;
}

void LocalDataPoolManager::performPeriodicHkGeneration(HkReceiver& receiver) {
  sid_t sid = receiver.dataId.sid;
  LocalPoolDataSetBase* dataSet = HasLocalDpIFManagerAttorney::getDataSetHandle(owner, sid);
  if (dataSet == nullptr) {
    printWarningOrError(sif::OutputTypes::OUT_WARNING, "performPeriodicHkGeneration",
                        DATASET_NOT_FOUND);
    return;
  }

  if (not LocalPoolDataSetAttorney::getReportingEnabled(*dataSet)) {
    return;
  }

  PeriodicHousekeepingHelper* periodicHelper =
      LocalPoolDataSetAttorney::getPeriodicHelper(*dataSet);

  if (periodicHelper == nullptr) {
    /* Configuration error */
    return;
  }

  if (not periodicHelper->checkOpNecessary()) {
    return;
  }

  ReturnValue_t result = generateHousekeepingPacket(sid, dataSet, true);
  if (result != returnvalue::OK) {
    /* Configuration error */
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "LocalDataPoolManager::performPeriodicHkOperation: HK generation failed."
                 << std::endl;
#else
    sif::printWarning("LocalDataPoolManager::performPeriodicHkOperation: HK generation failed.\n");
#endif
  }
}

ReturnValue_t LocalDataPoolManager::togglePeriodicGeneration(sid_t sid, bool enable,
                                                             bool isDiagnostics) {
  LocalPoolDataSetBase* dataSet = HasLocalDpIFManagerAttorney::getDataSetHandle(owner, sid);
  if (dataSet == nullptr) {
    printWarningOrError(sif::OutputTypes::OUT_WARNING, "togglePeriodicGeneration",
                        DATASET_NOT_FOUND);
    return DATASET_NOT_FOUND;
  }

  if ((LocalPoolDataSetAttorney::isDiagnostics(*dataSet) and not isDiagnostics) or
      (not LocalPoolDataSetAttorney::isDiagnostics(*dataSet) and isDiagnostics)) {
    return WRONG_HK_PACKET_TYPE;
  }

  if ((LocalPoolDataSetAttorney::getReportingEnabled(*dataSet) and enable) or
      (not LocalPoolDataSetAttorney::getReportingEnabled(*dataSet) and not enable)) {
    return REPORTING_STATUS_UNCHANGED;
  }

  LocalPoolDataSetAttorney::setReportingEnabled(*dataSet, enable);
  return returnvalue::OK;
}

ReturnValue_t LocalDataPoolManager::changeCollectionInterval(sid_t sid, float newCollectionInterval,
                                                             bool isDiagnostics) {
  LocalPoolDataSetBase* dataSet = HasLocalDpIFManagerAttorney::getDataSetHandle(owner, sid);
  if (dataSet == nullptr) {
    printWarningOrError(sif::OutputTypes::OUT_WARNING, "changeCollectionInterval",
                        DATASET_NOT_FOUND);
    return DATASET_NOT_FOUND;
  }

  bool targetIsDiagnostics = LocalPoolDataSetAttorney::isDiagnostics(*dataSet);
  if ((targetIsDiagnostics and not isDiagnostics) or (not targetIsDiagnostics and isDiagnostics)) {
    return WRONG_HK_PACKET_TYPE;
  }

  PeriodicHousekeepingHelper* periodicHelper =
      LocalPoolDataSetAttorney::getPeriodicHelper(*dataSet);

  if (periodicHelper == nullptr) {
    /* Configuration error, set might not have a corresponding pool manager */
    return PERIODIC_HELPER_INVALID;
  }

  periodicHelper->changeCollectionInterval(newCollectionInterval);
  return returnvalue::OK;
}

ReturnValue_t LocalDataPoolManager::generateSetStructurePacket(sid_t sid, bool isDiagnostics) {
  /* Get and check dataset first. */
  LocalPoolDataSetBase* dataSet = HasLocalDpIFManagerAttorney::getDataSetHandle(owner, sid);
  if (dataSet == nullptr) {
    printWarningOrError(sif::OutputTypes::OUT_WARNING, "performPeriodicHkGeneration",
                        DATASET_NOT_FOUND);
    return DATASET_NOT_FOUND;
  }

  bool targetIsDiagnostics = LocalPoolDataSetAttorney::isDiagnostics(*dataSet);
  if ((targetIsDiagnostics and not isDiagnostics) or (not targetIsDiagnostics and isDiagnostics)) {
    return WRONG_HK_PACKET_TYPE;
  }

  bool valid = dataSet->isValid();
  bool reportingEnabled = LocalPoolDataSetAttorney::getReportingEnabled(*dataSet);
  float collectionInterval =
      LocalPoolDataSetAttorney::getPeriodicHelper(*dataSet)->getCollectionIntervalInSeconds();

  // Generate set packet which can be serialized.
  HousekeepingSetPacket setPacket(sid, reportingEnabled, valid, collectionInterval, dataSet);
  size_t expectedSize = setPacket.getSerializedSize();
  uint8_t* storePtr = nullptr;
  store_address_t storeId;
  ReturnValue_t result = ipcStore->getFreeElement(&storeId, expectedSize, &storePtr);
  if (result != returnvalue::OK) {
    printWarningOrError(sif::OutputTypes::OUT_ERROR, "generateSetStructurePacket",
                        returnvalue::FAILED, "Could not get free element from IPC store.");
    return result;
  }

  // Serialize set packet into store.
  size_t size = 0;
  result = setPacket.serialize(&storePtr, &size, expectedSize, SerializeIF::Endianness::BIG);
  if (result != returnvalue::OK) {
    ipcStore->deleteData(storeId);
    return result;
  }
  if (expectedSize != size) {
    printWarningOrError(sif::OutputTypes::OUT_WARNING, "generateSetStructurePacket",
                        returnvalue::FAILED, "Expected size is not equal to serialized size");
  }

  // Send structure reporting reply.
  CommandMessage reply;
  if (isDiagnostics) {
    HousekeepingMessage::setDiagnosticsStuctureReportReply(&reply, sid, storeId);
  } else {
    HousekeepingMessage::setHkStuctureReportReply(&reply, sid, storeId);
  }

  result = hkQueue->reply(&reply);
  if (result != returnvalue::OK) {
    ipcStore->deleteData(storeId);
  }
  return result;
}

void LocalDataPoolManager::clearReceiversList() {
  /* Clear the vector completely and releases allocated memory. */
  HkReceivers().swap(hkReceivers);
  /* Also clear the reset helper if it exists */
  HkUpdateResetList().swap(hkUpdateResetList);
}

MutexIF* LocalDataPoolManager::getLocalPoolMutex() { return this->mutex; }

object_id_t LocalDataPoolManager::getCreatorObjectId() const { return owner->getObjectId(); }

void LocalDataPoolManager::printWarningOrError(sif::OutputTypes outputType,
                                               const char* functionName, ReturnValue_t error,
                                               const char* errorPrint) {
#if FSFW_VERBOSE_LEVEL >= 1
  if (errorPrint == nullptr) {
    if (error == DATASET_NOT_FOUND) {
      errorPrint = "Dataset not found";
    } else if (error == POOLOBJECT_NOT_FOUND) {
      errorPrint = "Pool Object not found";
    } else if (error == returnvalue::FAILED) {
      if (outputType == sif::OutputTypes::OUT_WARNING) {
        errorPrint = "Generic Warning";
      } else {
        errorPrint = "Generic error";
      }
    } else if (error == QUEUE_OR_DESTINATION_INVALID) {
      errorPrint = "Queue or destination not set";
    } else if (error == localpool::POOL_ENTRY_TYPE_CONFLICT) {
      errorPrint = "Pool entry type conflict";
    } else if (error == localpool::POOL_ENTRY_NOT_FOUND) {
      errorPrint = "Pool entry not found";
    } else {
      errorPrint = "Unknown error";
    }
  }
  object_id_t objectId = 0xffffffff;
  if (owner != nullptr) {
    objectId = owner->getObjectId();
  }

  if (outputType == sif::OutputTypes::OUT_WARNING) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "LocalDataPoolManager::" << functionName << ": Object ID 0x" << std::setw(8)
                 << std::setfill('0') << std::hex << objectId << " | " << errorPrint << std::dec
                 << std::setfill(' ') << std::endl;
#else
    sif::printWarning("LocalDataPoolManager::%s: Object ID 0x%08x | %s\n", functionName, objectId,
                      errorPrint);
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
  } else if (outputType == sif::OutputTypes::OUT_ERROR) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "LocalDataPoolManager::" << functionName << ": Object ID 0x" << std::setw(8)
               << std::setfill('0') << std::hex << objectId << " | " << errorPrint << std::dec
               << std::setfill(' ') << std::endl;
#else
    sif::printError("LocalDataPoolManager::%s: Object ID 0x%08x | %s\n", functionName, objectId,
                    errorPrint);
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
  }
#endif /* #if FSFW_VERBOSE_LEVEL >= 1 */
}

LocalDataPoolManager* LocalDataPoolManager::getPoolManagerHandle() { return this; }

void LocalDataPoolManager::setHkDestinationId(MessageQueueId_t hkDestId) {
  hkDestinationId = hkDestId;
}

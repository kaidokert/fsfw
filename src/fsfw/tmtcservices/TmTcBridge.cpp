#include "fsfw/tmtcservices/TmTcBridge.h"

#include "fsfw/globalfunctions/arrayprinter.h"
#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface.h"

#define TMTCBRIDGE_WIRETAPPING 0

TmTcBridge::TmTcBridge(object_id_t objectId, object_id_t tcDestination, object_id_t tmStoreId,
                       object_id_t tcStoreId)
    : SystemObject(objectId),
      tmStoreId(tmStoreId),
      tcStoreId(tcStoreId),
      tcDestination(tcDestination)

{
  tmTcReceptionQueue = QueueFactory::instance()->createMessageQueue(TMTC_RECEPTION_QUEUE_DEPTH);
}

TmTcBridge::~TmTcBridge() { QueueFactory::instance()->deleteMessageQueue(tmTcReceptionQueue); }

ReturnValue_t TmTcBridge::setNumberOfSentPacketsPerCycle(uint8_t sentPacketsPerCycle_) {
  if (sentPacketsPerCycle <= LIMIT_STORED_DATA_SENT_PER_CYCLE) {
    this->sentPacketsPerCycle = sentPacketsPerCycle_;
    return RETURN_OK;
  } else {
    FSFW_LOGW(
        "setNumberOfSentPacketsPerCycle: Number of packets sent per cycle exceeds limits. "
        "Keeping default value\n");
    return RETURN_FAILED;
  }
}

ReturnValue_t TmTcBridge::setMaxNumberOfPacketsStored(uint8_t maxNumberOfPacketsStored_) {
  if (maxNumberOfPacketsStored <= LIMIT_DOWNLINK_PACKETS_STORED) {
    this->maxNumberOfPacketsStored = maxNumberOfPacketsStored_;
    return RETURN_OK;
  } else {
    FSFW_LOGW(
        "setMaxNumberOfPacketsStored: Passed number of packets {} stored exceeds "
        "limit {}\nKeeping default value\n",
        maxNumberOfPacketsStored_, LIMIT_DOWNLINK_PACKETS_STORED);
    return RETURN_FAILED;
  }
}

ReturnValue_t TmTcBridge::initialize() {
  tcStore = ObjectManager::instance()->get<StorageManagerIF>(tcStoreId);
  if (tcStore == nullptr) {
    FSFW_LOGE("initialize: TC store invalid. Make sure it is created and set up properly\n");
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }
  tmStore = ObjectManager::instance()->get<StorageManagerIF>(tmStoreId);
  if (tmStore == nullptr) {
    FSFW_LOGE("initialize: TM store invalid. Make sure it is created and set up properly\n");
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }
  auto* tcDistributor = ObjectManager::instance()->get<AcceptsTelecommandsIF>(tcDestination);
  if (tcDistributor == nullptr) {
    FSFW_LOGE("initialize: TC Distributor invalid\n");
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }

  tmFifo = new DynamicFIFO<store_address_t>(maxNumberOfPacketsStored);

  tmTcReceptionQueue->setDefaultDestination(tcDistributor->getRequestQueue());
  return RETURN_OK;
}

ReturnValue_t TmTcBridge::performOperation(uint8_t operationCode) {
  ReturnValue_t result;
  result = handleTc();
  if (result != RETURN_OK) {
    FSFW_LOGWT("performOperation: Error handling TCs, code {}\n", result);
  }
  result = handleTm();
  if (result != RETURN_OK) {
    FSFW_LOGWT("performOperation: Error handling TMs, code {}\n", result);
  }
  return result;
}

ReturnValue_t TmTcBridge::handleTc() { return HasReturnvaluesIF::RETURN_OK; }

ReturnValue_t TmTcBridge::handleTm() {
  ReturnValue_t status = HasReturnvaluesIF::RETURN_OK;
  ReturnValue_t result = handleTmQueue();
  if (result != RETURN_OK) {
    FSFW_LOGET("handleTm: Error handling TM queue with error code {:#04x}\n", result);
    status = result;
  }

  if (tmStored and communicationLinkUp and (packetSentCounter < sentPacketsPerCycle)) {
    result = handleStoredTm();
    if (result != RETURN_OK) {
      FSFW_LOGE("handleTm: Error handling stored TMs\n");
      status = result;
    }
  }
  packetSentCounter = 0;
  return status;
}

ReturnValue_t TmTcBridge::handleTmQueue() {
  TmTcMessage message;
  const uint8_t* data = nullptr;
  size_t size = 0;
  ReturnValue_t status = HasReturnvaluesIF::RETURN_OK;
  for (ReturnValue_t result = tmTcReceptionQueue->receiveMessage(&message);
       result == HasReturnvaluesIF::RETURN_OK;
       result = tmTcReceptionQueue->receiveMessage(&message)) {
#if FSFW_VERBOSE_LEVEL >= 3
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Sent packet counter: " << static_cast<int>(packetSentCounter) << std::endl;
#else
    sif::printInfo("Sent packet counter: %d\n", packetSentCounter);
#endif
#endif /* FSFW_VERBOSE_LEVEL >= 3 */

    if (!communicationLinkUp or packetSentCounter >= sentPacketsPerCycle) {
      storeDownlinkData(&message);
      continue;
    }

    result = tmStore->getData(message.getStorageId(), &data, &size);
    if (result != HasReturnvaluesIF::RETURN_OK) {
      status = result;
      continue;
    }

    result = sendTm(data, size);
    if (result != HasReturnvaluesIF::RETURN_OK) {
      status = result;
    } else {
      tmStore->deleteData(message.getStorageId());
      packetSentCounter++;
    }
  }
  return status;
}

ReturnValue_t TmTcBridge::storeDownlinkData(TmTcMessage* message) {
  store_address_t storeId = 0;
  if (tmFifo == nullptr) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }

  if (tmFifo->full()) {
    FSFW_LOGWT(
        "storeDownlinkData: TM downlink max. number "
        "of stored packet IDs reached\n");
    if (overwriteOld) {
      tmFifo->retrieve(&storeId);
      tmStore->deleteData(storeId);
    } else {
      return HasReturnvaluesIF::RETURN_FAILED;
    }
  }

  storeId = message->getStorageId();
  tmFifo->insert(storeId);
  tmStored = true;
  return RETURN_OK;
}

ReturnValue_t TmTcBridge::handleStoredTm() {
  ReturnValue_t status = RETURN_OK;
  while (not tmFifo->empty() and packetSentCounter < sentPacketsPerCycle) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    // sif::info << "TMTC Bridge: Sending stored TM data. There are "
    //      << (int) tmFifo->size() << " left to send\r\n" << std::flush;
#endif

    store_address_t storeId;
    const uint8_t* data = nullptr;
    size_t size = 0;
    tmFifo->retrieve(&storeId);
    ReturnValue_t result = tmStore->getData(storeId, &data, &size);
    if (result != HasReturnvaluesIF::RETURN_OK) {
      status = result;
    }

    result = sendTm(data, size);
    if (result != RETURN_OK) {
      FSFW_LOGW("handleStoredTm: Could not send stored downlink data, code {:#04x}\n", result);
      status = result;
    }
    packetSentCounter++;

    if (tmFifo->empty()) {
      tmStored = false;
    }
    tmStore->deleteData(storeId);
  }
  return status;
}

void TmTcBridge::registerCommConnect() {
  if (not communicationLinkUp) {
    communicationLinkUp = true;
  }
}

void TmTcBridge::registerCommDisconnect() {
  if (communicationLinkUp) {
    communicationLinkUp = false;
  }
}

MessageQueueId_t TmTcBridge::getReportReceptionQueue(uint8_t virtualChannel) {
  return tmTcReceptionQueue->getId();
}

void TmTcBridge::printData(uint8_t* data, size_t dataLen) { arrayprinter::print(data, dataLen); }

uint16_t TmTcBridge::getIdentifier() {
  // This is no PUS service, so we just return 0
  return 0;
}

MessageQueueId_t TmTcBridge::getRequestQueue() {
  // Default implementation: Relay TC messages to TC distributor directly.
  return tmTcReceptionQueue->getDefaultDestination();
}

void TmTcBridge::setFifoToOverwriteOldData(bool overwriteOld) { this->overwriteOld = overwriteOld; }

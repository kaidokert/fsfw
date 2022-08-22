#include "fsfw/tmtcservices/TmTcBridge.h"

#include "fsfw/globalfunctions/arrayprinter.h"
#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

#define TMTCBRIDGE_WIRETAPPING 0

TmTcBridge::TmTcBridge(const char* name, object_id_t objectId, object_id_t tcDestination,
                       object_id_t tmStoreId, object_id_t tcStoreId)
    : SystemObject(objectId),
      name(name),
      tmStoreId(tmStoreId),
      tcStoreId(tcStoreId),
      tcDestination(tcDestination)

{
  tmTcReceptionQueue = QueueFactory::instance()->createMessageQueue(TMTC_RECEPTION_QUEUE_DEPTH);
}

TmTcBridge::~TmTcBridge() { QueueFactory::instance()->deleteMessageQueue(tmTcReceptionQueue); }

ReturnValue_t TmTcBridge::setNumberOfSentPacketsPerCycle(uint8_t sentPacketsPerCycle) {
  if (sentPacketsPerCycle <= LIMIT_STORED_DATA_SENT_PER_CYCLE) {
    this->sentPacketsPerCycle = sentPacketsPerCycle;
    return returnvalue::OK;
  } else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "TmTcBridge::setNumberOfSentPacketsPerCycle: Number of "
                 << "packets sent per cycle exceeds limits. "
                 << "Keeping default value." << std::endl;
#endif
    return returnvalue::FAILED;
  }
}

ReturnValue_t TmTcBridge::setMaxNumberOfPacketsStored(uint8_t maxNumberOfPacketsStored) {
  if (maxNumberOfPacketsStored <= LIMIT_DOWNLINK_PACKETS_STORED) {
    this->maxNumberOfPacketsStored = maxNumberOfPacketsStored;
    return returnvalue::OK;
  } else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "TmTcBridge::setMaxNumberOfPacketsStored: Number of "
                 << "packets stored exceeds limits. "
                 << "Keeping default value." << std::endl;
#endif
    return returnvalue::FAILED;
  }
}

ReturnValue_t TmTcBridge::initialize() {
  tcStore = ObjectManager::instance()->get<StorageManagerIF>(tcStoreId);
  if (tcStore == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "TmTcBridge::initialize: TC store invalid. Make sure"
                  "it is created and set up properly."
               << std::endl;
#endif
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }
  tmStore = ObjectManager::instance()->get<StorageManagerIF>(tmStoreId);
  if (tmStore == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "TmTcBridge::initialize: TM store invalid. Make sure"
                  "it is created and set up properly."
               << std::endl;
#endif
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }
  auto* tcDistributor = ObjectManager::instance()->get<AcceptsTelecommandsIF>(tcDestination);
  if (tcDistributor == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "TmTcBridge::initialize: TC Distributor invalid" << std::endl;
#endif
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }

  tmFifo = new DynamicFIFO<store_address_t>(maxNumberOfPacketsStored);

  tmTcReceptionQueue->setDefaultDestination(tcDistributor->getRequestQueue());
  return returnvalue::OK;
}

ReturnValue_t TmTcBridge::performOperation(uint8_t operationCode) {
  ReturnValue_t result;
  result = handleTc();
  if (result != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "TmTcBridge::performOperation: "
               << "Error handling TCs" << std::endl;
#endif
  }
  result = handleTm();
  if (result != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "TmTcBridge::performOperation: "
               << "Error handling TMs" << std::endl;
#endif
  }
  return result;
}

ReturnValue_t TmTcBridge::handleTc() { return returnvalue::OK; }

ReturnValue_t TmTcBridge::handleTm() {
  ReturnValue_t status = returnvalue::OK;
  ReturnValue_t result = handleTmQueue();
  if (result != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "TmTcBridge::handleTm: Error handling TM queue with error code 0x" << std::hex
               << result << std::dec << "!" << std::endl;
#endif
    status = result;
  }

  if (tmStored and communicationLinkUp and (packetSentCounter < sentPacketsPerCycle)) {
    result = handleStoredTm();
    if (result != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "TmTcBridge::handleTm: Error handling stored TMs!" << std::endl;
#endif
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
  ReturnValue_t status = returnvalue::OK;
  for (ReturnValue_t result = tmTcReceptionQueue->receiveMessage(&message);
       result == returnvalue::OK; result = tmTcReceptionQueue->receiveMessage(&message)) {
#if FSFW_VERBOSE_LEVEL >= 3
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::info << "Sent packet counter: " << static_cast<int>(packetSentCounter) << std::endl;
#else
    sif::printInfo("Sent packet counter: %d\n", packetSentCounter);
#endif
#endif /* FSFW_VERBOSE_LEVEL >= 3 */

    if (communicationLinkUp == false or packetSentCounter >= sentPacketsPerCycle) {
      storeDownlinkData(&message);
      continue;
    }

    result = tmStore->getData(message.getStorageId(), &data, &size);
    if (result != returnvalue::OK) {
      status = result;
      continue;
    }

    result = sendTm(data, size);
    if (result != returnvalue::OK) {
      status = result;
    } else {
      tmStore->deleteData(message.getStorageId());
      packetSentCounter++;
    }
  }
  return status;
}

ReturnValue_t TmTcBridge::storeDownlinkData(TmTcMessage* message) {
  store_address_t storeId;
  if (tmFifo == nullptr) {
    return returnvalue::FAILED;
  }

  if (tmFifo->full()) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "TmTcBridge::storeDownlinkData: TM downlink max. number "
                    "of stored packet IDs reached!"
                 << std::endl;
#else
    sif::printWarning(
        "TmTcBridge::storeDownlinkData: TM downlink max. number "
        "of stored packet IDs reached!\n");
#endif
    if (overwriteOld) {
      tmFifo->retrieve(&storeId);
      tmStore->deleteData(storeId);
    } else {
      return returnvalue::FAILED;
    }
  }

  storeId = message->getStorageId();
  tmFifo->insert(storeId);
  tmStored = true;
  return returnvalue::OK;
}

ReturnValue_t TmTcBridge::handleStoredTm() {
  ReturnValue_t status = returnvalue::OK;
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
    if (result != returnvalue::OK) {
      status = result;
    }

    result = sendTm(data, size);
    if (result != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "TMTC Bridge: Could not send stored downlink data" << std::endl;
#endif
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

uint32_t TmTcBridge::getIdentifier() const {
  // This is no PUS service, so we just return 0
  return 0;
}

MessageQueueId_t TmTcBridge::getRequestQueue() const {
  // Default implementation: Relay TC messages to TC distributor directly.
  return tmTcReceptionQueue->getDefaultDestination();
}

void TmTcBridge::setFifoToOverwriteOldData(bool overwriteOld) { this->overwriteOld = overwriteOld; }

const char* TmTcBridge::getName() const { return name; }

#include "fsfw/tcdistribution/PusDistributor.h"

#include "definitions.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tcdistribution/CCSDSDistributorIF.h"
#include "fsfw/tmtcservices/PusVerificationReport.h"

#define PUS_DISTRIBUTOR_DEBUGGING 0

PusDistributor::PusDistributor(uint16_t setApid, object_id_t setObjectId,
                               CCSDSDistributorIF* distributor, StorageManagerIF* store_)
    : TcDistributor(setObjectId),
      store(store_),
      checker(setApid, ccsds::PacketType::TC),
      ccsdsDistributor(distributor),
      tcStatus(returnvalue::FAILED) {}

PusDistributor::~PusDistributor() = default;

PusDistributor::TcMqMapIter PusDistributor::selectDestination() {
#if FSFW_CPP_OSTREAM_ENABLED == 1 && PUS_DISTRIBUTOR_DEBUGGING == 1
  store_address_t storeId = currentMessage.getStorageId();
  sif::debug << "PUSDistributor::handlePacket received: " << storeId.poolIndex << ", "
             << storeId.packetIndex << std::endl;
#endif
  auto queueMapIt = queueMap.end();
  // TODO: Need to set the data
  const uint8_t* packetPtr = nullptr;
  size_t packetLen = 0;
  if (store->getData(currentMessage.getStorageId(), &packetPtr, &packetLen) != returnvalue::OK) {
    return queueMapIt;
  }
  ReturnValue_t result = reader.setReadOnlyData(packetPtr, packetLen);
  if (result != returnvalue::OK) {
    tcStatus = PACKET_LOST;
    return queueMapIt;
  }
  // CRC check done by checker
  result = reader.parseDataWithoutCrcCheck();
  if (result != returnvalue::OK) {
    tcStatus = PACKET_LOST;
    return queueMapIt;
  }
  if (reader.getFullData() != nullptr) {
    tcStatus = checker.checkPacket(reader, reader.getFullPacketLen());
    if (tcStatus != returnvalue::OK) {
      checkerFailurePrinter();
    }
    uint32_t queue_id = reader.getService();
    queueMapIt = queueMap.find(queue_id);
  } else {
    tcStatus = PACKET_LOST;
  }

  if (queueMapIt == this->queueMap.end()) {
    tcStatus = DESTINATION_NOT_FOUND;
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "PUSDistributor::handlePacket: Destination not found" << std::endl;
#else
    sif::printDebug("PUSDistributor::handlePacket: Destination not found\n");
#endif /* !FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif
  }

  if (tcStatus != returnvalue::OK) {
    return this->queueMap.end();
  } else {
    return queueMapIt;
  }
}

ReturnValue_t PusDistributor::registerService(AcceptsTelecommandsIF* service) {
  uint16_t serviceId = service->getIdentifier();
#if PUS_DISTRIBUTOR_DEBUGGING == 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::info << "Service ID: " << static_cast<int>(serviceId) << std::endl;
#else
  sif::printInfo("Service ID: %d\n", static_cast<int>(serviceId));
#endif
#endif
  MessageQueueId_t queue = service->getRequestQueue();
  auto returnPair = queueMap.emplace(serviceId, queue);
  if (not returnPair.second) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "PUSDistributor::registerService: Service ID already"
                  " exists in map"
               << std::endl;
#else
    sif::printError("PUSDistributor::registerService: Service ID already exists in map\n");
#endif
#endif
    return SERVICE_ID_ALREADY_EXISTS;
  }
  return returnvalue::OK;
}

MessageQueueId_t PusDistributor::getRequestQueue() const { return tcQueue->getId(); }

ReturnValue_t PusDistributor::callbackAfterSending(ReturnValue_t queueStatus) {
  if (queueStatus != returnvalue::OK) {
    tcStatus = queueStatus;
  }
  if (tcStatus != returnvalue::OK) {
    verifyChannel->sendFailureReport({tcverif::ACCEPTANCE_FAILURE, reader, tcStatus});
    // A failed packet is deleted immediately after reporting,
    // otherwise it will block memory.
    store->deleteData(currentMessage.getStorageId());
    return returnvalue::FAILED;
  } else {
    verifyChannel->sendSuccessReport({tcverif::ACCEPTANCE_SUCCESS, reader});
    return returnvalue::OK;
  }
}

uint32_t PusDistributor::getIdentifier() const { return checker.getApid(); }

ReturnValue_t PusDistributor::initialize() {
  if (store == nullptr) {
    store = ObjectManager::instance()->get<StorageManagerIF>(objects::TC_STORE);
    if (store == nullptr) {
      return ObjectManagerIF::CHILD_INIT_FAILED;
    }
  }
  if (ccsdsDistributor == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "PUSDistributor::initialize: Packet source invalid" << std::endl;
    sif::error << " Make sure it exists and implements CCSDSDistributorIF!" << std::endl;
#else
    sif::printError("PusDistributor::initialize: Packet source invalid\n");
    sif::printError("Make sure it exists and implements CCSDSDistributorIF\n");
#endif
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }
  if (verifyChannel == nullptr) {
    verifyChannel =
        ObjectManager::instance()->get<VerificationReporterIF>(objects::VERIFICATION_REPORTER);
    if (verifyChannel == nullptr) {
      return ObjectManagerIF::CHILD_INIT_FAILED;
    }
  }
  return ccsdsDistributor->registerApplication(this);
}

void PusDistributor::checkerFailurePrinter() const {
#if FSFW_VERBOSE_LEVEL >= 1
  const char* keyword = "unnamed error";
  if (tcStatus == tcdistrib::INCORRECT_CHECKSUM) {
    keyword = "checksum";
  } else if (tcStatus == tcdistrib::INCORRECT_PRIMARY_HEADER) {
    keyword = "incorrect primary header";
  } else if (tcStatus == tcdistrib::INVALID_APID) {
    keyword = "illegal APID";
  } else if (tcStatus == tcdistrib::INCORRECT_SECONDARY_HEADER) {
    keyword = "incorrect secondary header";
  } else if (tcStatus == tcdistrib::INCOMPLETE_PACKET) {
    keyword = "incomplete packet";
  }
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::warning << "PUSDistributor::handlePacket: Packet format invalid, " << keyword << " error"
               << std::endl;
#else
  sif::printWarning("PUSDistributor::handlePacket: Packet format invalid, %s error\n", keyword);
#endif
#endif
}

#include "fsfw/tcdistribution/PusDistributor.h"

#include "definitions.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tcdistribution/CcsdsDistributorIF.h"
#include "fsfw/tmtcservices/PusVerificationReport.h"

#define PUS_DISTRIBUTOR_DEBUGGING 0

PusDistributor::PusDistributor(uint16_t setApid, object_id_t setObjectId,
                               CcsdsDistributorIF* distributor, StorageManagerIF* store_)
    : TcDistributorBase(setObjectId),
      store(store_),
      checker(setApid, ccsds::PacketType::TC),
      ccsdsDistributor(distributor),
      tcStatus(returnvalue::FAILED) {}

PusDistributor::~PusDistributor() = default;

ReturnValue_t PusDistributor::selectDestination(MessageQueueId_t& destId) {
#if FSFW_CPP_OSTREAM_ENABLED == 1 && PUS_DISTRIBUTOR_DEBUGGING == 1
  store_address_t storeId = currentMessage.getStorageId();
  sif::debug << "PUSDistributor::handlePacket received: " << storeId.poolIndex << ", "
             << storeId.packetIndex << std::endl;
#endif
  // TODO: Need to set the data
  const uint8_t* packetPtr = nullptr;
  size_t packetLen = 0;
  ReturnValue_t result =
      store->getData(currentMessage.getStorageId(), &packetPtr, &packetLen) != returnvalue::OK;
  if (result != returnvalue::OK) {
    tcStatus = PACKET_LOST;
    return result;
  }
  result = reader.setReadOnlyData(packetPtr, packetLen);
  if (result != returnvalue::OK) {
    tcStatus = PACKET_LOST;
    return result;
  }
  // CRC check done by checker
  result = reader.parseDataWithoutCrcCheck();
  if (result != returnvalue::OK) {
    tcStatus = PACKET_LOST;
    return result;
  }

  if (reader.getFullData() != nullptr) {
    tcStatus = checker.checkPacket(reader, reader.getFullPacketLen());
    if (tcStatus != returnvalue::OK) {
      checkerFailurePrinter();
    }
    uint8_t pusId = reader.getService();
    auto iter = receiverMap.find(pusId);
    if (iter == receiverMap.end()) {
      tcStatus = DESTINATION_NOT_FOUND;
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::debug << "PUSDistributor::handlePacket: Destination not found" << std::endl;
#else
      sif::printDebug("PUSDistributor::handlePacket: Destination not found\n");
#endif /* !FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif
    }
    destId = iter->second.destId;
  } else {
    tcStatus = PACKET_LOST;
  }
  return tcStatus;
}

ReturnValue_t PusDistributor::registerService(const AcceptsTelecommandsIF& service) {
  uint16_t serviceId = service.getIdentifier();
#if PUS_DISTRIBUTOR_DEBUGGING == 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::info << "Service ID: " << static_cast<int>(serviceId) << std::endl;
#else
  sif::printInfo("Service ID: %d\n", static_cast<int>(serviceId));
#endif
#endif
  MessageQueueId_t queue = service.getRequestQueue();
  auto returnPair = receiverMap.emplace(serviceId, ServiceInfo(service.getName(), queue));
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
    sif::printError("Make sure it exists and implements CcsdsDistributorIF\n");
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
  return ccsdsDistributor->registerApplication(CcsdsDistributorIF::DestInfo(*this, false));
}

void PusDistributor::checkerFailurePrinter() const {
#if FSFW_VERBOSE_LEVEL >= 1
  const char* reason = "Unknown reason";
  if (tcStatus == tmtcdistrib::INCORRECT_CHECKSUM) {
    reason = "Checksum Error";
  } else if (tcStatus == tmtcdistrib::INCORRECT_PRIMARY_HEADER) {
    reason = "Incorrect Primary Header";
  } else if (tcStatus == tmtcdistrib::INVALID_APID) {
    reason = "Illegal APID";
  } else if (tcStatus == tmtcdistrib::INCORRECT_SECONDARY_HEADER) {
    reason = "Incorrect Secondary Header";
  } else if (tcStatus == tmtcdistrib::INCOMPLETE_PACKET) {
    reason = "Incomplete packet";
  }
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::warning << "PUSDistributor::handlePacket: Check failed: " << reason << std::endl;
#else
  sif::printWarning("PUSDistributor::handlePacket: Check failed: %s\n", reason);
#endif
#endif
}

const char* PusDistributor::getName() const { return "PUS Distributor"; }

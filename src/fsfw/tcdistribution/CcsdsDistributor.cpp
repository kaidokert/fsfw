#include "fsfw/tcdistribution/CcsdsDistributor.h"

#include "definitions.h"
#include "fsfw/FSFW.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tmtcpacket/ccsds/SpacePacketReader.h"

#define CCSDS_DISTRIBUTOR_DEBUGGING 0

CcsdsDistributor::CcsdsDistributor(uint16_t setDefaultApid, object_id_t setObjectId,
                                   StorageManagerIF* tcStore, MessageQueueIF* queue,
                                   CcsdsPacketCheckIF* packetChecker)
    : TcDistributorBase(setObjectId, queue),
      defaultApid(setDefaultApid),
      tcStore(tcStore),
      packetChecker(packetChecker) {}

CcsdsDistributor::~CcsdsDistributor() {
  if (ownedPacketChecker) {
    delete packetChecker;
  }
}

ReturnValue_t CcsdsDistributor::selectDestination(MessageQueueId_t& destId) {
#if CCSDS_DISTRIBUTOR_DEBUGGING == 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::debug << "CCSDSDistributor::selectDestination received: "
             << this->currentMessage.getStorageId().poolIndex << ", "
             << this->currentMessage.getStorageId().packetIndex << std::endl;
#else
  sif::printDebug("CCSDSDistributor::selectDestination received: %d, %d\n",
                  currentMessage.getStorageId().poolIndex,
                  currentMessage.getStorageId().packetIndex);
#endif
#endif
  auto accessorPair = tcStore->getData(currentMessage.getStorageId());
  ReturnValue_t result = accessorPair.first;
  if (result != returnvalue::OK) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "CCSDSDistributor::selectDestination: Getting data from"
                  " store failed!"
               << std::endl;
#else
    sif::printError(
        "CCSDSDistributor::selectDestination: Getting data from"
        " store failed!\n");
#endif
#endif
    return result;
  }
  // Minimum length of a space packet
  if (accessorPair.second.size() < ccsds::HEADER_LEN + 1) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << __func__ << ": SP with length" << accessorPair.second.size() << " too short"
               << std::endl;
#else
    sif::printError("%s: SP with length %d too short\n", __func__, accessorPair.second.size());
#endif
#endif
    return SerializeIF::STREAM_TOO_SHORT;
  }
  SpacePacketReader currentPacket(accessorPair.second.data(), accessorPair.second.size());
  result = packetChecker->checkPacket(currentPacket, accessorPair.second.size());
  if (result != returnvalue::OK) {
    handlePacketCheckFailure(result);
    return result;
  }
#if FSFW_CPP_OSTREAM_ENABLED == 1 && CCSDS_DISTRIBUTOR_DEBUGGING == 1
  sif::info << "CCSDSDistributor::selectDestination has packet with APID 0x" << std::hex
            << currentPacket.getApid() << std::dec << std::endl;
#endif
  auto iter = receiverMap.find(currentPacket.getApid());
  if (iter != receiverMap.end()) {
    destId = iter->second.destId;
  } else if (iter == receiverMap.end()) {
    // The APID was not found. Forward packet to main SW-APID anyway to
    // create acceptance failure report.
    iter = receiverMap.find(defaultApid);
    if (iter != receiverMap.end()) {
      destId = iter->second.destId;
    } else {
      return DESTINATION_NOT_FOUND;
    }
  }
  if (iter->second.removeHeader) {
    // Do not call accessor release method here to ensure the old packet gets deleted.
    return handleCcsdsHeaderRemoval(accessorPair.second);
  }
  accessorPair.second.release();
  return returnvalue::OK;
}

void CcsdsDistributor::handlePacketCheckFailure(ReturnValue_t result) {
#if FSFW_VERBOSE_LEVEL >= 1
  const char* reason = "Unknown reason";
  if (result == tmtcdistrib::INVALID_CCSDS_VERSION) {
    reason = "Invalid CCSDS version";
  } else if (result == tmtcdistrib::INCOMPLETE_PACKET) {
    reason = "Size missmatch between CCSDS  data length and packet length";
  } else if (result == tmtcdistrib::INVALID_APID) {
    reason = "No valid handler APID found";
  } else if (result == tmtcdistrib::INVALID_PACKET_TYPE) {
    reason = "Invalid Packet Type TM detected";
  }
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::warning << "CCSDS packet check failed: " << reason << std::endl;
#else
  sif::printWarning("CCSDS packet check failed: %s\n", reason);
#endif
#endif
}

MessageQueueId_t CcsdsDistributor::getRequestQueue() const { return tcQueue->getId(); }

ReturnValue_t CcsdsDistributor::registerApplication(DestInfo info) {
  ReturnValue_t returnValue = returnvalue::OK;
  auto insertPair = receiverMap.emplace(info.apid, info);
  if (not insertPair.second) {
    returnValue = returnvalue::FAILED;
  }
  return returnValue;
}

uint32_t CcsdsDistributor::getIdentifier() const { return 0; }

ReturnValue_t CcsdsDistributor::initialize() {
  ReturnValue_t result = TcDistributorBase::initialize();
  if (result != returnvalue::OK) {
    return result;
  }
  if (packetChecker == nullptr) {
    ownedPacketChecker = true;
    packetChecker = new CcsdsPacketChecker(ccsds::PacketType::TC);
  }
  if (tcStore == nullptr) {
    tcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::TC_STORE);
    if (tcStore == nullptr) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "CCSDSDistributor::initialize: Could not initialize"
                    " TC store!"
                 << std::endl;
#else
      sif::printError(
          "CCSDSDistributor::initialize: Could not initialize"
          " TC store!\n");
#endif
#endif
      return ObjectManagerIF::CHILD_INIT_FAILED;
    }
  }
  return result;
}

ReturnValue_t CcsdsDistributor::callbackAfterSending(ReturnValue_t queueStatus) {
  if (queueStatus != returnvalue::OK) {
    tcStore->deleteData(currentMessage.getStorageId());
  }
  return returnvalue::OK;
}

void CcsdsDistributor::print() {
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::debug << "Distributor content is: " << std::endl << "ID\t| Message Queue ID" << std::endl;
  sif::debug << std::setfill('0') << std::setw(8) << std::hex;
  for (const auto& iter : receiverMap) {
    sif::debug << iter.first << "\t| 0x" << iter.second.destId
               << ", Header Removed: " << std::boolalpha << iter.second.removeHeader << std::endl;
  }
  sif::debug << std::setfill(' ') << std::dec;
#endif
}

const char* CcsdsDistributor::getName() const { return "CCSDS Distributor"; }

ReturnValue_t CcsdsDistributor::handleCcsdsHeaderRemoval(ConstStorageAccessor& accessor) {
  store_address_t newStoreId;
  ReturnValue_t result = tcStore->addData(&newStoreId, accessor.data() + ccsds::HEADER_LEN,
                                          accessor.size() - ccsds::HEADER_LEN);
  if (result != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << __func__ << ": TC store full" << std::endl;
#else
    sif::printError("%s: TC store full\n", __func__);
#endif
    return result;
  }
  currentMessage.setStorageId(newStoreId);
  // The const accessor will delete the old data automatically
  return returnvalue::OK;
}

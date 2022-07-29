#include "fsfw/tcdistribution/CcsdsDistributor.h"

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tmtcpacket/ccsds/SpacePacketReader.h"

#define CCSDS_DISTRIBUTOR_DEBUGGING 0

CcsdsDistributor::CcsdsDistributor(uint16_t setDefaultApid, object_id_t setObjectId,
                                   CcsdsPacketCheckIF* packetChecker)
    : TcDistributor(setObjectId), defaultApid(setDefaultApid), packetChecker(packetChecker) {}

CcsdsDistributor::~CcsdsDistributor() = default;

TcDistributor::TcMqMapIter CcsdsDistributor::selectDestination() {
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
  const uint8_t* packet = nullptr;
  size_t size = 0;
  ReturnValue_t result = tcStore->getData(currentMessage.getStorageId(), &packet, &size);
  if (result != HasReturnvaluesIF::RETURN_OK) {
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
    return queueMap.end();
  }
  SpacePacketReader currentPacket(packet, size);
  result = packetChecker->checkPacket(currentPacket, size);
  if (result != HasReturnvaluesIF::RETURN_OK) {
  }
#if FSFW_CPP_OSTREAM_ENABLED == 1 && CCSDS_DISTRIBUTOR_DEBUGGING == 1
  sif::info << "CCSDSDistributor::selectDestination has packet with APID 0x" << std::hex
            << currentPacket.getApid() << std::dec << std::endl;
#endif
  auto position = this->queueMap.find(currentPacket.getApid());
  if (position != this->queueMap.end()) {
    return position;
  } else {
    // The APID was not found. Forward packet to main SW-APID anyway to
    //  create acceptance failure report.
    return queueMap.find(this->defaultApid);
  }
}

MessageQueueId_t CcsdsDistributor::getRequestQueue() { return tcQueue->getId(); }

ReturnValue_t CcsdsDistributor::registerApplication(AcceptsTelecommandsIF* application) {
  ReturnValue_t returnValue = RETURN_OK;
  auto insertPair =
      this->queueMap.emplace(application->getIdentifier(), application->getRequestQueue());
  if (not insertPair.second) {
    returnValue = RETURN_FAILED;
  }
  return returnValue;
}

ReturnValue_t CcsdsDistributor::registerApplication(uint16_t apid, MessageQueueId_t id) {
  ReturnValue_t returnValue = RETURN_OK;
  auto insertPair = this->queueMap.emplace(apid, id);
  if (not insertPair.second) {
    returnValue = RETURN_FAILED;
  }
  return returnValue;
}

uint16_t CcsdsDistributor::getIdentifier() { return 0; }

ReturnValue_t CcsdsDistributor::initialize() {
  if (packetChecker == nullptr) {
    packetChecker = new CcsdsPacketChecker(ccsds::PacketType::TC);
  }
  ReturnValue_t status = this->TcDistributor::initialize();
  this->tcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::TC_STORE);
  if (this->tcStore == nullptr) {
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
    status = RETURN_FAILED;
  }
  return status;
}

ReturnValue_t CcsdsDistributor::callbackAfterSending(ReturnValue_t queueStatus) {
  if (queueStatus != RETURN_OK) {
    tcStore->deleteData(currentMessage.getStorageId());
  }
  return RETURN_OK;
}

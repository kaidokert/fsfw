#include "fsfw/tcdistribution/CCSDSDistributor.h"

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface.h"
#include "fsfw/tmtcpacket/SpacePacketBase.h"

#define CCSDS_DISTRIBUTOR_DEBUGGING 0

CCSDSDistributor::CCSDSDistributor(uint16_t setDefaultApid, object_id_t setObjectId)
    : TcDistributor(setObjectId), defaultApid(setDefaultApid) {}

CCSDSDistributor::~CCSDSDistributor() {}

TcDistributor::TcMqMapIter CCSDSDistributor::selectDestination() {
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
  ReturnValue_t result = this->tcStore->getData(currentMessage.getStorageId(), &packet, &size);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    FSFW_LOGWT("{}", "selectDestination: Getting data from store failed");
  }
  SpacePacketBase currentPacket(packet);

#if FSFW_CPP_OSTREAM_ENABLED == 1 && CCSDS_DISTRIBUTOR_DEBUGGING == 1
  sif::info << "CCSDSDistributor::selectDestination has packet with APID " << std::hex
            << currentPacket.getAPID() << std::dec << std::endl;
#endif
  TcMqMapIter position = this->queueMap.find(currentPacket.getAPID());
  if (position != this->queueMap.end()) {
    return position;
  } else {
    // The APID was not found. Forward packet to main SW-APID anyway to
    //  create acceptance failure report.
    return this->queueMap.find(this->defaultApid);
  }
}

MessageQueueId_t CCSDSDistributor::getRequestQueue() { return tcQueue->getId(); }

ReturnValue_t CCSDSDistributor::registerApplication(AcceptsTelecommandsIF* application) {
  ReturnValue_t returnValue = RETURN_OK;
  auto insertPair =
      this->queueMap.emplace(application->getIdentifier(), application->getRequestQueue());
  if (not insertPair.second) {
    returnValue = RETURN_FAILED;
  }
  return returnValue;
}

ReturnValue_t CCSDSDistributor::registerApplication(uint16_t apid, MessageQueueId_t id) {
  ReturnValue_t returnValue = RETURN_OK;
  auto insertPair = this->queueMap.emplace(apid, id);
  if (not insertPair.second) {
    returnValue = RETURN_FAILED;
  }
  return returnValue;
}

uint16_t CCSDSDistributor::getIdentifier() { return 0; }

ReturnValue_t CCSDSDistributor::initialize() {
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

ReturnValue_t CCSDSDistributor::callbackAfterSending(ReturnValue_t queueStatus) {
  if (queueStatus != RETURN_OK) {
    tcStore->deleteData(currentMessage.getStorageId());
  }
  return RETURN_OK;
}

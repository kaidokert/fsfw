#include "fsfw/tcdistribution/CFDPDistributor.h"

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface.h"
#include "fsfw/tcdistribution/CCSDSDistributorIF.h"
#include "fsfw/tmtcpacket/cfdp/CFDPPacketStored.h"

#ifndef FSFW_CFDP_DISTRIBUTOR_DEBUGGING
#define FSFW_CFDP_DISTRIBUTOR_DEBUGGING 1
#endif

CFDPDistributor::CFDPDistributor(uint16_t setApid, object_id_t setObjectId,
                                 object_id_t setPacketSource)
    : TcDistributor(setObjectId),
      apid(setApid),
      checker(setApid),
      tcStatus(RETURN_FAILED),
      packetSource(setPacketSource) {}

CFDPDistributor::~CFDPDistributor() {}

CFDPDistributor::TcMqMapIter CFDPDistributor::selectDestination() {
#if FSFW_CFDP_DISTRIBUTOR_DEBUGGING == 1
  store_address_t storeId = this->currentMessage.getStorageId();
  FSFW_LOGI("selectDestination was called with pool index {} and packet index {}\n",
            storeId.poolIndex, storeId.packetIndex);
#endif
  TcMqMapIter queueMapIt = this->queueMap.end();
  if (this->currentPacket == nullptr) {
    return queueMapIt;
  }
  this->currentPacket->setStoreAddress(this->currentMessage.getStorageId());
  if (currentPacket->getWholeData() != nullptr) {
    tcStatus = checker.checkPacket(currentPacket);
    if (tcStatus != HasReturnvaluesIF::RETURN_OK) {
      FSFW_LOGWT("selectDestination: Packet format invalid, code {}\n", static_cast<int>(tcStatus));
    }
    queueMapIt = this->queueMap.find(0);
  } else {
    tcStatus = PACKET_LOST;
  }

  if (queueMapIt == this->queueMap.end()) {
    tcStatus = DESTINATION_NOT_FOUND;
    FSFW_LOGWT("{}", "handlePacket: Destination not found\n");
  }

  if (tcStatus != RETURN_OK) {
    return this->queueMap.end();
  } else {
    return queueMapIt;
  }
}

ReturnValue_t CFDPDistributor::registerHandler(AcceptsTelecommandsIF* handler) {
  uint16_t handlerId =
      handler->getIdentifier();  // should be 0, because CFDPHandler does not set a set a service-ID
  FSFW_LOGIT("CFDPDistributor::registerHandler: Handler ID {}\n", static_cast<int>(handlerId));
  MessageQueueId_t queue = handler->getRequestQueue();
  auto returnPair = queueMap.emplace(handlerId, queue);
  if (not returnPair.second) {
    FSFW_LOGE("{}", "CFDPDistributor::registerHandler: Service ID already exists in map\n");
    return SERVICE_ID_ALREADY_EXISTS;
  }
  return HasReturnvaluesIF::RETURN_OK;
}

MessageQueueId_t CFDPDistributor::getRequestQueue() { return tcQueue->getId(); }

// ReturnValue_t CFDPDistributor::callbackAfterSending(ReturnValue_t queueStatus) {
//     if (queueStatus != RETURN_OK) {
//         tcStatus = queueStatus;
//     }
//     if (tcStatus != RETURN_OK) {
//         this->verifyChannel.sendFailureReport(tc_verification::ACCEPTANCE_FAILURE,
//                 currentPacket, tcStatus);
//         // A failed packet is deleted immediately after reporting,
//         // otherwise it will block memory.
//         currentPacket->deletePacket();
//         return RETURN_FAILED;
//     } else {
//         this->verifyChannel.sendSuccessReport(tc_verification::ACCEPTANCE_SUCCESS,
//                 currentPacket);
//         return RETURN_OK;
//     }
// }

uint16_t CFDPDistributor::getIdentifier() { return this->apid; }

ReturnValue_t CFDPDistributor::initialize() {
  currentPacket = new CFDPPacketStored();
  if (currentPacket == nullptr) {
    // Should not happen, memory allocation failed!
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }

  auto* ccsdsDistributor = ObjectManager::instance()->get<CCSDSDistributorIF>(packetSource);
  if (ccsdsDistributor == nullptr) {
    FSFW_LOGE("{}", "initialize: Packet source invalid. Does it implement CCSDSDistributorIF?\n");
    return RETURN_FAILED;
  }
  return ccsdsDistributor->registerApplication(this);
}

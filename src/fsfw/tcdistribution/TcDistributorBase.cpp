#include "fsfw/tcdistribution/TcDistributorBase.h"

#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/tmtcservices/TmTcMessage.h"

TcDistributorBase::TcDistributorBase(object_id_t objectId, MessageQueueIF* tcQueue_)
    : SystemObject(objectId), tcQueue(tcQueue_) {
  if (tcQueue == nullptr) {
    ownedQueue = true;
    tcQueue = QueueFactory::instance()->createMessageQueue(DISTRIBUTER_MAX_PACKETS);
  }
}

TcDistributorBase::~TcDistributorBase() {
  if (ownedQueue) {
    QueueFactory::instance()->deleteMessageQueue(tcQueue);
  }
}

ReturnValue_t TcDistributorBase::performOperation(uint8_t opCode) {
  ReturnValue_t status;
  ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
  for (status = tcQueue->receiveMessage(&currentMessage); status == RETURN_OK;
       status = tcQueue->receiveMessage(&currentMessage)) {
    ReturnValue_t packetResult = handlePacket();
    if (packetResult != HasReturnvaluesIF::RETURN_OK) {
      result = packetResult;
      triggerEvent(HANDLE_PACKET_FAILED, packetResult, __LINE__);
    }
  }
  if (status == MessageQueueIF::EMPTY) {
    return result;
  }
  return result;
}

ReturnValue_t TcDistributorBase::handlePacket() {
  MessageQueueId_t destId;
  ReturnValue_t result = selectDestination(destId);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  return callbackAfterSending(tcQueue->sendMessage(destId, &currentMessage));
}

ReturnValue_t TcDistributorBase::callbackAfterSending(ReturnValue_t queueStatus) {
  return RETURN_OK;
}

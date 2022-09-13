#include "TcDistributorBase.h"

#include "definitions.h"
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
  ReturnValue_t result = returnvalue::OK;
  for (status = tcQueue->receiveMessage(&currentMessage); status == returnvalue::OK;
       status = tcQueue->receiveMessage(&currentMessage)) {
    ReturnValue_t packetResult = handlePacket();
    if (packetResult != returnvalue::OK) {
      result = packetResult;
      triggerEvent(tmtcdistrib::HANDLE_PACKET_FAILED, packetResult, 1);
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
  if (result != returnvalue::OK) {
    return result;
  }
  return callbackAfterSending(tcQueue->sendMessage(destId, &currentMessage));
}

ReturnValue_t TcDistributorBase::callbackAfterSending(ReturnValue_t queueStatus) {
  return returnvalue::OK;
}

#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/tcdistribution/TcDistributorBase.h"
#include "fsfw/tmtcservices/TmTcMessage.h"

TcDistributorBase::TcDistributorBase(object_id_t objectId) : SystemObject(objectId) {
  tcQueue = QueueFactory::instance()->createMessageQueue(DISTRIBUTER_MAX_PACKETS);
}

TcDistributorBase::~TcDistributorBase() { QueueFactory::instance()->deleteMessageQueue(tcQueue); }

ReturnValue_t TcDistributorBase::performOperation(uint8_t opCode) {
  ReturnValue_t status;
  for (status = tcQueue->receiveMessage(&currentMessage); status == RETURN_OK;
       status = tcQueue->receiveMessage(&currentMessage)) {
    status = handlePacket();
  }
  if (status == MessageQueueIF::EMPTY) {
    return RETURN_OK;
  }
  return status;
}

ReturnValue_t TcDistributorBase::handlePacket() {
  MessageQueueId_t destId;
  ReturnValue_t result = selectDestination(destId);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  result = tcQueue->sendMessage(destId, &currentMessage);
  return callbackAfterSending(result);
}

ReturnValue_t TcDistributorBase::callbackAfterSending(ReturnValue_t queueStatus) {
  return RETURN_OK;
}

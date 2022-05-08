#include "fsfw/tcdistribution/TcDistributor.h"

#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/serviceinterface.h"
#include "fsfw/tmtcservices/TmTcMessage.h"

TcDistributor::TcDistributor(object_id_t objectId) : SystemObject(objectId) {
  tcQueue = QueueFactory::instance()->createMessageQueue(DISTRIBUTER_MAX_PACKETS);
}

TcDistributor::~TcDistributor() { QueueFactory::instance()->deleteMessageQueue(tcQueue); }

ReturnValue_t TcDistributor::performOperation(uint8_t opCode) {
  ReturnValue_t status = RETURN_OK;
  for (status = tcQueue->receiveMessage(&currentMessage); status == RETURN_OK;
       status = tcQueue->receiveMessage(&currentMessage)) {
    status = handlePacket();
  }
  if (status == MessageQueueIF::EMPTY) {
    return RETURN_OK;
  } else {
    return status;
  }
}

ReturnValue_t TcDistributor::handlePacket() {
  TcMqMapIter queueMapIt = this->selectDestination();
  ReturnValue_t returnValue = RETURN_FAILED;
  if (queueMapIt != this->queueMap.end()) {
    returnValue = this->tcQueue->sendMessage(queueMapIt->second, &this->currentMessage);
  }
  return this->callbackAfterSending(returnValue);
}

void TcDistributor::print() {
  FSFW_LOGI("{}", "Distributor content is:\nID\t| Message Queue ID");
  for (const auto& queueMapIter : queueMap) {
    FSFW_LOGI("{} \t| {:#010x}", queueMapIter.first, queueMapIter.second);
  }
}

ReturnValue_t TcDistributor::callbackAfterSending(ReturnValue_t queueStatus) { return RETURN_OK; }

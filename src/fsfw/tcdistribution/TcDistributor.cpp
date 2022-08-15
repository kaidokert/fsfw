#include "fsfw/tcdistribution/TcDistributor.h"

#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tmtcservices/TmTcMessage.h"

TcDistributor::TcDistributor(object_id_t objectId) : SystemObject(objectId) {
  tcQueue = QueueFactory::instance()->createMessageQueue(DISTRIBUTER_MAX_PACKETS);
}

TcDistributor::~TcDistributor() { QueueFactory::instance()->deleteMessageQueue(tcQueue); }

ReturnValue_t TcDistributor::performOperation(uint8_t opCode) {
  ReturnValue_t status;
  for (status = tcQueue->receiveMessage(&currentMessage); status == returnvalue::OK;
       status = tcQueue->receiveMessage(&currentMessage)) {
    status = handlePacket();
  }
  if (status == MessageQueueIF::EMPTY) {
    return returnvalue::OK;
  }
  return status;
}

ReturnValue_t TcDistributor::handlePacket() {
  auto queueMapIt = selectDestination();
  ReturnValue_t result = returnvalue::FAILED;
  if (queueMapIt != queueMap.end()) {
    result = tcQueue->sendMessage(queueMapIt->second, &currentMessage);
  }
  return callbackAfterSending(result);
}

void TcDistributor::print() {
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::debug << "Distributor content is: " << std::endl << "ID\t| Message Queue ID" << std::endl;
  sif::debug << std::setfill('0') << std::setw(8) << std::hex;
  for (const auto& queueMapIter : queueMap) {
    sif::debug << queueMapIter.first << "\t| 0x" << queueMapIter.second << std::endl;
  }
  sif::debug << std::setfill(' ') << std::dec;
#endif
}

ReturnValue_t TcDistributor::callbackAfterSending(ReturnValue_t queueStatus) {
  return returnvalue::OK;
}

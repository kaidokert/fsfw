#include "TcDistributor.h"

#include "../serviceinterface/ServiceInterfaceStream.h"
#include "../tmtcservices/TmTcMessage.h"
#include "../ipc/QueueFactory.h"

TcDistributor::TcDistributor(object_id_t objectId) :
		SystemObject(objectId) {
	tcQueue = QueueFactory::instance()->
	        createMessageQueue(DISTRIBUTER_MAX_PACKETS);
}

TcDistributor::~TcDistributor() {
	QueueFactory::instance()->deleteMessageQueue(tcQueue);
}

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
		returnValue = this->tcQueue->sendMessage(queueMapIt->second,
				&this->currentMessage);
	}
	return this->callbackAfterSending(returnValue);
}

void TcDistributor::print() {
	sif::debug << "Distributor content is: " << std::endl
	        << "ID\t| Message Queue ID" << std::endl;
	sif::debug << std::setfill('0') << std::setw(8) << std::hex;
	for (const auto& queueMapIter: queueMap) {
		sif::debug << queueMapIter.first << "\t| 0x"   << queueMapIter.second
		         << std::endl;
	}
	sif::debug << std::setfill(' ') << std::dec;

}

ReturnValue_t TcDistributor::callbackAfterSending(ReturnValue_t queueStatus) {
	return RETURN_OK;
}

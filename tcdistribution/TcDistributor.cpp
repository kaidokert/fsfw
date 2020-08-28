#include "../serviceinterface/ServiceInterfaceStream.h"
#include "../serviceinterface/ServiceInterfaceStream.h"
#include "../tcdistribution/TcDistributor.h"
#include "../tmtcservices/TmTcMessage.h"
#include "../ipc/QueueFactory.h"

TcDistributor::TcDistributor(object_id_t set_object_id) :
		SystemObject(set_object_id), tcQueue(NULL) {
	tcQueue = QueueFactory::instance()->createMessageQueue(DISTRIBUTER_MAX_PACKETS);
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
	sif::debug << "Distributor content is: " << std::endl << "ID\t| message queue id"
			<< std::endl;
	for (TcMqMapIter it = this->queueMap.begin(); it != this->queueMap.end();
			it++) {
		sif::debug << it->first << "\t| 0x" << std::hex << it->second << std::dec
				<< std::endl;
	}
	sif::debug << std::dec;

}

ReturnValue_t TcDistributor::callbackAfterSending(ReturnValue_t queueStatus) {
	return RETURN_OK;
}

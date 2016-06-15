#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/ipc/MessageQueue.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tcdistribution/TcDistributor.h>
#include <framework/tmtcservices/TmTcMessage.h>

TcDistributor::TcDistributor(object_id_t set_object_id) :
		SystemObject(set_object_id), tcQueue(DISTRIBUTER_MAX_PACKETS) {

}

TcDistributor::~TcDistributor() {
	//Nothing to do in the destructor, MQ's are destroyed elsewhere.
}

ReturnValue_t TcDistributor::performOperation(void) {
	ReturnValue_t status = RETURN_OK;
//	debug << "TcDistributor: performing Operation." << std::endl;
	for (status = tcQueue.receiveMessage(&currentMessage); status == RETURN_OK;
			status = tcQueue.receiveMessage(&currentMessage)) {
		status = handlePacket();
	}
	if (status == OSAL::QUEUE_EMPTY) {
		return RETURN_OK;
	} else {
		return status;
	}
}

ReturnValue_t TcDistributor::handlePacket() {

	iterator_t queueMapIt = this->selectDestination();
	ReturnValue_t returnValue = RETURN_FAILED;
	if (queueMapIt != this->queueMap.end()) {
		returnValue = this->tcQueue.sendMessage(queueMapIt->second,
				&this->currentMessage);
	}
	return this->callbackAfterSending(returnValue);
}

void TcDistributor::print() {
	debug << "Distributor content is: " << std::endl << "ID\t| message queue id"
			<< std::endl;
	for (iterator_t it = this->queueMap.begin(); it != this->queueMap.end();
			it++) {
		debug << it->first << "\t| 0x" << std::hex << it->second << std::dec
				<< std::endl;
	}
	debug << std::dec;

}

ReturnValue_t TcDistributor::callbackAfterSending(ReturnValue_t queueStatus) {
	return RETURN_OK;
}

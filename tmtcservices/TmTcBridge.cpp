#include <framework/tmtcservices/TmTcBridge.h>

#include <framework/ipc/QueueFactory.h>
#include <framework/objectmanager/ObjectManagerIF.h>
#include <framework/osal/FreeRTOS/PeriodicTask.h>
#include <framework/tmtcservices/AcceptsTelecommandsIF.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tasks/PeriodicTaskIF.h>

TmTcBridge::TmTcBridge(object_id_t objectId_,
        object_id_t ccsdsPacketDistributor_): SystemObject(objectId_),
        ccsdsPacketDistributor(ccsdsPacketDistributor_),
		sentPacketsPerCycle(5), delayBetweenSentPacketsMs(0)
{
	    TmTcReceptionQueue = QueueFactory::instance()->
			createMessageQueue(TMTC_RECEPTION_QUEUE_DEPTH);
}

TmTcBridge::~TmTcBridge() {}

void TmTcBridge::setDelayBetweenSentPackets(uint32_t delayBetweenSentPackets) {
	this->delayBetweenSentPacketsMs = delayBetweenSentPackets;
}

ReturnValue_t TmTcBridge::setNumberOfSentPacketsPerCycle(
		uint8_t sentPacketsPerCycle) {
	if(sentPacketsPerCycle <= MAX_STORED_DATA_SENT_PER_CYCLE) {
		this->sentPacketsPerCycle = sentPacketsPerCycle;
		return RETURN_OK;
	}
	else {
		return RETURN_FAILED;
	}
}

ReturnValue_t TmTcBridge::initialize() {
	tcStore = objectManager->get<StorageManagerIF>(objects::TC_STORE);
	if (tcStore == NULL) {
		return RETURN_FAILED;
	}
	tmStore = objectManager->get<StorageManagerIF>(objects::TM_STORE);
	if (tmStore == NULL) {
		return RETURN_FAILED;
	}
	AcceptsTelecommandsIF* tcDistributor =
			objectManager->get<AcceptsTelecommandsIF>(ccsdsPacketDistributor);
	if (tcDistributor == NULL) {
		return RETURN_FAILED;
	}
	TmTcReceptionQueue->setDefaultDestination(tcDistributor->getRequestQueue());
	return RETURN_OK;
}

ReturnValue_t TmTcBridge::performOperation(uint8_t operationCode) {
	ReturnValue_t result;
	result = handleTc();
	if(result != RETURN_OK) {
		error << "TMTC Bridge: Error handling TCs" << std::endl;
	}
	result = handleTm();
	if (result != RETURN_OK) {
		error << "TMTC Bridge: Error handling TMs" << std::endl;
	}
	return result;
}

ReturnValue_t TmTcBridge::handleTc() {
	uint8_t * recvBuffer = nullptr;
	size_t recvLen = 0;
	ReturnValue_t result = receiveTc(&recvBuffer, &recvLen);
	if(result == RETURN_OK and recvLen > 0 and recvBuffer != nullptr) {
		store_address_t storeId = 0;
		ReturnValue_t result = tcStore->addData(&storeId,
				recvBuffer, (uint32_t)recvLen);
		if(result != RETURN_OK) {
			return result;
		}
		TmTcMessage message(storeId);
		if (TmTcReceptionQueue->sendToDefault(&message) != RETURN_OK) {
			tcStore->deleteData(storeId);
		}
	}
	return result;
}

ReturnValue_t TmTcBridge::handleTm() {
	ReturnValue_t result = readTmQueue();
	if(result != RETURN_OK) {
		error << "TMTC Bridge: Reading TM Queue failed" << std::endl;
		return RETURN_FAILED;
	}

	if(tmStored && communicationLinkUp) {
		result = sendStoredTm();
	}
	return result;

}

ReturnValue_t TmTcBridge::readTmQueue() {
	TmTcMessage message;
	const uint8_t* data = nullptr;
	uint32_t size = 0;
	for (ReturnValue_t result = TmTcReceptionQueue->receiveMessage(&message);
		 result == RETURN_OK; result = TmTcReceptionQueue->receiveMessage(&message))
	{
		if(communicationLinkUp == false) {
			result = storeDownlinkData(&message);
			return result;
		}

		result = tmStore->getData(message.getStorageId(), &data, &size);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			continue;
		}

		result = sendTm(data, size);
		if (result != RETURN_OK) {
			error << "TMTC Bridge: Could not send TM packet"<< std::endl;
			tmStore->deleteData(message.getStorageId());
			return result;

		}
		tmStore->deleteData(message.getStorageId());
	}
	return RETURN_OK;
}

ReturnValue_t TmTcBridge::storeDownlinkData(TmTcMessage *message) {
	//debug << "TMTC Bridge: Comm Link down. "
	//		 "Saving packet ID to be sent later\r\n" << std::flush;
	store_address_t storeId = 0;

	if(fifo.full()) {
		error << "TMTC Bridge: TM downlink max. number of stored packet IDs "
				 "reached! Overwriting old data" << std::endl;
		fifo.retrieve(&storeId);
		tmStore->deleteData(storeId);
	}
	storeId = message->getStorageId();
	fifo.insert(storeId);
	tmStored = true;
	return RETURN_OK;
}

ReturnValue_t TmTcBridge::sendStoredTm() {
	uint8_t counter = 0;
	ReturnValue_t result = RETURN_OK;
	while(!fifo.empty() && counter < sentPacketsPerCycle) {
		//info << "TMTC Bridge: Sending stored TM data. There are "
		//     << (int) fifo.size() << " left to send\r\n" << std::flush;
		store_address_t storeId;
		const uint8_t* data = NULL;
		uint32_t size = 0;
		fifo.retrieve(&storeId);
		result = tmStore->getData(storeId, &data, &size);
		// This does not work yet: is not static function
		//PeriodicTaskIF::sleepFor(delayBetweenSentPacketsMs);
		sendTm(data,size);
		if(result != RETURN_OK) {
			error << "TMTC Bridge: Could not send stored downlink data"
			      << std::endl;
			result = RETURN_FAILED;
		}
		counter ++;

		if(fifo.empty()) {
			tmStored = false;
		}
		tmStore->deleteData(storeId);
	}
	return result;
}

void TmTcBridge::registerCommConnect() {
	if(!communicationLinkUp) {
		info << "TMTC Bridge: Registered Comm Link Connect" << std::endl;
		communicationLinkUp = true;
	}
}

void TmTcBridge::registerCommDisconnect() {
	info << "TMTC Bridge: Registered Comm Link Disconnect" << std::endl;
	if(communicationLinkUp) {
		communicationLinkUp = false;
	}
}

MessageQueueId_t TmTcBridge::getReportReceptionQueue(uint8_t virtualChannel) {
	return TmTcReceptionQueue->getId();
}

void TmTcBridge::printData(uint8_t * data, size_t dataLen) {
	info << "TMTC Bridge: Printing data: [";
	for(uint32_t i=0;i<dataLen;i++) {
		info << std::hex << (int)data[i];
		if(i < dataLen-1){
			info << " , ";
		}
	}
	info << " ] " << std::endl;
}

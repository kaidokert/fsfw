#include <framework/tmtcservices/TmTcBridge.h>

#include <framework/ipc/QueueFactory.h>
#include <framework/tmtcservices/AcceptsTelecommandsIF.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/globalfunctions/arrayprinter.h>

TmTcBridge::TmTcBridge(object_id_t objectId,
        object_id_t ccsdsPacketDistributor): SystemObject(objectId),
        ccsdsPacketDistributor(ccsdsPacketDistributor)
{
    TmTcReceptionQueue = QueueFactory::instance()->
            createMessageQueue(TMTC_RECEPTION_QUEUE_DEPTH);
}

TmTcBridge::~TmTcBridge() {}

ReturnValue_t TmTcBridge::setNumberOfSentPacketsPerCycle(
		uint8_t sentPacketsPerCycle) {
	if(sentPacketsPerCycle <= LIMIT_STORED_DATA_SENT_PER_CYCLE) {
		this->sentPacketsPerCycle = sentPacketsPerCycle;
		return RETURN_OK;
	}
	else {
	    sif::warning << "TmTcBridge: Number of packets sent per cycle "
	               "exceeds limits. Keeping default value." << std::endl;
		return RETURN_FAILED;
	}
}

ReturnValue_t TmTcBridge::setMaxNumberOfPacketsStored(
        uint8_t maxNumberOfPacketsStored) {
    if(maxNumberOfPacketsStored <= LIMIT_DOWNLINK_PACKETS_STORED) {
        this->maxNumberOfPacketsStored = maxNumberOfPacketsStored;
        return RETURN_OK;
    }
    else {
        sif::warning << "TmTcBridge: Number of packets stored "
                   "exceeds limits. Keeping default value." << std::endl;
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
		sif::error << "TMTC Bridge: Error handling TCs" << std::endl;
	}
	result = handleTm();
	if (result != RETURN_OK) {
		sif::error << "TMTC Bridge: Error handling TMs" << std::endl;
	}
	return result;
}

ReturnValue_t TmTcBridge::handleTc() {
	uint8_t * recvBuffer = nullptr;
	size_t recvLen = 0;
	ReturnValue_t result = receiveTc(&recvBuffer, &recvLen);
	return result;
}

ReturnValue_t TmTcBridge::handleTm() {
	ReturnValue_t result = handleTmQueue();
	if(result != RETURN_OK) {
		sif::error << "TMTC Bridge: Reading TM Queue failed" << std::endl;
		return RETURN_FAILED;
	}

	if(tmStored and communicationLinkUp) {
		result = handleStoredTm();
	}
	return result;

}

ReturnValue_t TmTcBridge::handleTmQueue() {
	TmTcMessage message;
	const uint8_t* data = nullptr;
	size_t size = 0;
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
			sif::error << "TMTC Bridge: Could not send TM packet"<< std::endl;
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

	if(tmFifo.full()) {
		sif::error << "TMTC Bridge: TM downlink max. number of stored packet IDs "
				 "reached! Overwriting old data" << std::endl;
		tmFifo.retrieve(&storeId);
		tmStore->deleteData(storeId);
	}
	storeId = message->getStorageId();
	tmFifo.insert(storeId);
	tmStored = true;
	return RETURN_OK;
}

ReturnValue_t TmTcBridge::handleStoredTm() {
	uint8_t counter = 0;
	ReturnValue_t result = RETURN_OK;
	while(not tmFifo.empty() and counter < sentPacketsPerCycle) {
		//info << "TMTC Bridge: Sending stored TM data. There are "
		//     << (int) fifo.size() << " left to send\r\n" << std::flush;
		store_address_t storeId;
		const uint8_t* data = nullptr;
		size_t size = 0;
		tmFifo.retrieve(&storeId);
		result = tmStore->getData(storeId, &data, &size);

		sendTm(data,size);

		if(result != RETURN_OK) {
			sif::error << "TMTC Bridge: Could not send stored downlink data"
			      << std::endl;
			result = RETURN_FAILED;
		}
		counter ++;

		if(tmFifo.empty()) {
			tmStored = false;
		}
		tmStore->deleteData(storeId);
	}
	return result;
}

void TmTcBridge::registerCommConnect() {
	if(not communicationLinkUp) {
		//info << "TMTC Bridge: Registered Comm Link Connect" << std::endl;
		communicationLinkUp = true;
	}
}

void TmTcBridge::registerCommDisconnect() {
	//info << "TMTC Bridge: Registered Comm Link Disconnect" << std::endl;
	if(communicationLinkUp) {
		communicationLinkUp = false;
	}
}

MessageQueueId_t TmTcBridge::getReportReceptionQueue(uint8_t virtualChannel) {
	return TmTcReceptionQueue->getId();
}


void TmTcBridge::printData(uint8_t * data, size_t dataLen) {
	arrayprinter::print(data, dataLen);
}

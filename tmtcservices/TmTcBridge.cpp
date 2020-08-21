#include "../tmtcservices/TmTcBridge.h"

#include "../ipc/QueueFactory.h"
#include "../tmtcservices/AcceptsTelecommandsIF.h"
#include "../serviceinterface/ServiceInterfaceStream.h"
#include "../globalfunctions/arrayprinter.h"

TmTcBridge::TmTcBridge(object_id_t objectId, object_id_t tcDestination,
		object_id_t tmStoreId, object_id_t tcStoreId):
		SystemObject(objectId),tmStoreId(tmStoreId), tcStoreId(tcStoreId),
		tcDestination(tcDestination)

{
    tmTcReceptionQueue = QueueFactory::instance()->
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
	    sif::warning << "TmTcBridge::setNumberOfSentPacketsPerCycle: Number of "
	             << "packets sent per cycle exceeds limits. "
	             << "Keeping default value." << std::endl;
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
        sif::warning << "TmTcBridge::setMaxNumberOfPacketsStored: Number of "
                << "packets stored exceeds limits. "
                << "Keeping default value." << std::endl;
        return RETURN_FAILED;
    }
}

ReturnValue_t TmTcBridge::initialize() {
	tcStore = objectManager->get<StorageManagerIF>(tcStoreId);
	if (tcStore == nullptr) {
		sif::error << "TmTcBridge::initialize: TC store invalid. Make sure"
				"it is created and set up properly." << std::endl;
		return ObjectManagerIF::CHILD_INIT_FAILED;
	}
	tmStore = objectManager->get<StorageManagerIF>(tmStoreId);
	if (tmStore == nullptr) {
		sif::error << "TmTcBridge::initialize: TM store invalid. Make sure"
				"it is created and set up properly." << std::endl;
		return ObjectManagerIF::CHILD_INIT_FAILED;
	}
	AcceptsTelecommandsIF* tcDistributor =
			objectManager->get<AcceptsTelecommandsIF>(tcDestination);
	if (tcDistributor == nullptr) {
		sif::error << "TmTcBridge::initialize: TC Distributor invalid"
				<< std::endl;
		return ObjectManagerIF::CHILD_INIT_FAILED;
	}

	tmTcReceptionQueue->setDefaultDestination(tcDistributor->getRequestQueue());
	return RETURN_OK;
}

ReturnValue_t TmTcBridge::performOperation(uint8_t operationCode) {
	ReturnValue_t result;
	result = handleTc();
	if(result != RETURN_OK) {
		sif::debug << "TmTcBridge::performOperation: "
		        << "Error handling TCs" << std::endl;
	}
	result = handleTm();
	if (result != RETURN_OK) {
	    sif::debug << "TmTcBridge::performOperation: "
	                    << "Error handling TMs" << std::endl;
	}
	return result;
}

ReturnValue_t TmTcBridge::handleTc() {
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t TmTcBridge::handleTm() {
	ReturnValue_t result = handleTmQueue();
	if(result != RETURN_OK) {
		sif::warning << "TmTcBridge: Reading TM Queue failed" << std::endl;
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
	ReturnValue_t status = HasReturnvaluesIF::RETURN_OK;
	for (ReturnValue_t result = tmTcReceptionQueue->receiveMessage(&message);
		 result == RETURN_OK; result = tmTcReceptionQueue->receiveMessage(&message))
	{
		if(communicationLinkUp == false) {
			storeDownlinkData(&message);
			continue;
		}

		result = tmStore->getData(message.getStorageId(), &data, &size);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			sif::error << "TmTcBridge::handleTmQueue: Invalid store "
					"ID!" << std::endl;
			continue;
		}

		result = sendTm(data, size);
		if (result != RETURN_OK) {
			sif::warning << "TmTcBridge: Could not send TM packet" << std::endl;
			status = HasReturnvaluesIF::RETURN_FAILED;
		}
		tmStore->deleteData(message.getStorageId());
	}
	return status;
}

// todo: make it configurable whether old data is deleted.
ReturnValue_t TmTcBridge::storeDownlinkData(TmTcMessage *message) {
	store_address_t storeId = 0;

	if(tmFifo.full()) {
		sif::error << "TmTcBridge::storeDownlinkData: TM downlink max. number "
		        << "of stored packet IDs reached! "
		        << "Overwriting old data" << std::endl;
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
	return tmTcReceptionQueue->getId();
}


void TmTcBridge::printData(uint8_t * data, size_t dataLen) {
	arrayprinter::print(data, dataLen);
}

uint16_t TmTcBridge::getIdentifier() {
	// This is no PUS service, so we just return 0
	return 0;
}

MessageQueueId_t TmTcBridge::getRequestQueue() {
	// Default implementation: Relay TC messages to TC distributor directly.
	return tmTcReceptionQueue->getDefaultDestination();
}

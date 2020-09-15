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

	tmFifo = new DynamicFIFO<store_address_t>(maxNumberOfPacketsStored);

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
    ReturnValue_t status = HasReturnvaluesIF::RETURN_OK;
	ReturnValue_t result = handleTmQueue();
	if(result != RETURN_OK) {
		sif::error << "TmTcBridge::handleTm: Error handling TM queue!"
		        << std::endl;
		status = result;
	}

	if(tmStored and communicationLinkUp and
	        (packetSentCounter < sentPacketsPerCycle)) {
	    result = handleStoredTm();
	    if(result != RETURN_OK) {
	        sif::error << "TmTcBridge::handleTm: Error handling stored TMs!"
	                << std::endl;
	        status = result;
	    }
	}
	packetSentCounter = 0;
	return status;
}

ReturnValue_t TmTcBridge::handleTmQueue() {
	TmTcMessage message;
	const uint8_t* data = nullptr;
	size_t size = 0;
	ReturnValue_t status = HasReturnvaluesIF::RETURN_OK;
	for (ReturnValue_t result = tmTcReceptionQueue->receiveMessage(&message);
		 result == HasReturnvaluesIF::RETURN_OK;
		 result = tmTcReceptionQueue->receiveMessage(&message))
	{
	    //sif::info << (int) packetSentCounter << std::endl;
		if(communicationLinkUp == false or
		        packetSentCounter >= sentPacketsPerCycle) {
			storeDownlinkData(&message);
			continue;
		}

		result = tmStore->getData(message.getStorageId(), &data, &size);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			status = result;
			continue;
		}

		result = sendTm(data, size);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			status = result;
		}
		else {
		    tmStore->deleteData(message.getStorageId());
		    packetSentCounter++;
		}
	}
	return status;
}

ReturnValue_t TmTcBridge::storeDownlinkData(TmTcMessage *message) {
	store_address_t storeId = 0;

	if(tmFifo->full()) {
	    sif::debug << "TmTcBridge::storeDownlinkData: TM downlink max. number "
	                    << "of stored packet IDs reached! " << std::endl;
	    if(overwriteOld) {
	        tmFifo->retrieve(&storeId);
	        tmStore->deleteData(storeId);
	    }
	    else {
	        return HasReturnvaluesIF::RETURN_FAILED;
	    }
	}

	storeId = message->getStorageId();
	tmFifo->insert(storeId);
	tmStored = true;
	return RETURN_OK;
}

ReturnValue_t TmTcBridge::handleStoredTm() {
    ReturnValue_t status = RETURN_OK;
	while(not tmFifo->empty() and packetSentCounter < sentPacketsPerCycle) {
		//sif::info << "TMTC Bridge: Sending stored TM data. There are "
		//     << (int) tmFifo->size() << " left to send\r\n" << std::flush;

		store_address_t storeId;
		const uint8_t* data = nullptr;
		size_t size = 0;
		tmFifo->retrieve(&storeId);
		ReturnValue_t result = tmStore->getData(storeId, &data, &size);
		if(result != HasReturnvaluesIF::RETURN_OK) {
		    status = result;
		}

		result = sendTm(data,size);
		if(result != RETURN_OK) {
			sif::error << "TMTC Bridge: Could not send stored downlink data"
			      << std::endl;
			status = result;
		}
		packetSentCounter ++;

		if(tmFifo->empty()) {
			tmStored = false;
		}
		tmStore->deleteData(storeId);
	}
	return status;
}

void TmTcBridge::registerCommConnect() {
	if(not communicationLinkUp) {
		//sif::info << "TMTC Bridge: Registered Comm Link Connect" << std::endl;
		communicationLinkUp = true;
	}
}

void TmTcBridge::registerCommDisconnect() {
	//sif::info << "TMTC Bridge: Registered Comm Link Disconnect" << std::endl;
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

void TmTcBridge::setFifoToOverwriteOldData(bool overwriteOld) {
    this->overwriteOld = overwriteOld;
}

#include "LocalDataPoolManager.h"
#include "LocalPoolDataSetBase.h"
#include "../housekeeping/AcceptsHkPacketsIF.h"
#include "../ipc/MutexFactory.h"
#include "../ipc/MutexHelper.h"
#include "../ipc/QueueFactory.h"

#include <array>
#include <cmath>

object_id_t LocalDataPoolManager::defaultHkDestination = objects::NO_OBJECT;

LocalDataPoolManager::LocalDataPoolManager(HasLocalDataPoolIF* owner,
        MessageQueueIF* queueToUse, bool appendValidityBuffer):
        appendValidityBuffer(appendValidityBuffer) {
	if(owner == nullptr) {
		sif::error << "HkManager: Invalid supplied owner!" << std::endl;
		return;
	}
	this->owner = owner;
	mutex = MutexFactory::instance()->createMutex();
	if(mutex == nullptr) {
	    sif::error << "LocalDataPoolManager::LocalDataPoolManager: "
	            "Could not create mutex." << std::endl;
	}
	ipcStore = objectManager->get<StorageManagerIF>(objects::IPC_STORE);
	if(ipcStore == nullptr) {
	    sif::error << "LocalDataPoolManager::LocalDataPoolManager: "
	            "Could not set IPC store." << std::endl;
	}

	hkQueue = queueToUse;

	if(defaultHkDestination != objects::NO_OBJECT) {
	    AcceptsHkPacketsIF* hkPacketReceiver =
	            objectManager->get<AcceptsHkPacketsIF>(defaultHkDestination);
	    if(hkPacketReceiver != nullptr) {
	        hkDestinationId = hkPacketReceiver->getHkQueue();
	    }
	}
}

LocalDataPoolManager::~LocalDataPoolManager() {}

ReturnValue_t LocalDataPoolManager::initialize(MessageQueueIF* queueToUse) {
    if(queueToUse == nullptr) {
        sif::error << "LocalDataPoolManager::initialize: Supplied queue "
                "invalid!" << std::endl;
    }
    hkQueue = queueToUse;

    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t LocalDataPoolManager::initializeAfterTaskCreation(
        uint8_t nonDiagInvlFactor) {
    setNonDiagnosticIntervalFactor(nonDiagInvlFactor);
    diagnosticMinimumInterval = owner->getPeriodicOperationFrequency();
    regularMinimumInterval = diagnosticMinimumInterval *
            nonDiagnosticIntervalFactor;
    return initializeHousekeepingPoolEntriesOnce();
}

ReturnValue_t LocalDataPoolManager::initializeHousekeepingPoolEntriesOnce() {
	if(not mapInitialized) {
		ReturnValue_t result = owner->initializeLocalDataPool(localPoolMap,
		        *this);
		if(result == HasReturnvaluesIF::RETURN_OK) {
			mapInitialized = true;
		}
		return result;
	}
	sif::warning << "HousekeepingManager: The map should only be initialized "
	        "once!" << std::endl;
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t LocalDataPoolManager::performHkOperation() {
    for(auto& hkReceiversIter: hkReceiversMap) {
        HkReceiver* receiver = &hkReceiversIter.second;

        switch(receiver->reportingType) {
        case(ReportingType::PERIODIC): {
            if(receiver->dataId.dataSetSid.notSet()) {
                // Periodic packets shall only be generated from datasets.
                continue;
            }
            performPeriodicHkGeneration(receiver);
            break;
        }
        case(ReportingType::UPDATE_SNAPSHOT): {
            // check whether data has changed and send messages in case it has.
            break;
        }
        default:
            // This should never happen.
            return HasReturnvaluesIF::RETURN_FAILED;
        }
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t LocalDataPoolManager::subscribeForPeriodicPacket(sid_t sid,
		bool enableReporting, float collectionInterval, bool isDiagnostics,
		object_id_t packetDestination) {
	AcceptsHkPacketsIF* hkReceiverObject =
			objectManager->get<AcceptsHkPacketsIF>(packetDestination);
	if(hkReceiverObject == nullptr) {
		sif::error << "LocalDataPoolManager::subscribeForPeriodicPacket:"
				" Invalid receiver!"<< std::endl;
		return HasReturnvaluesIF::RETURN_OK;
	}

	struct HkReceiver hkReceiver;
	hkReceiver.dataId.dataSetSid = sid;
	hkReceiver.reportingType = ReportingType::PERIODIC;
	hkReceiver.destinationQueue = hkReceiverObject->getHkQueue();

	if(not isDiagnostics) {
	    hkReceiver.hkParameter.collectionIntervalTicks =
	            intervalSecondsToInterval(isDiagnostics, collectionInterval *
	            nonDiagnosticIntervalFactor);
	}
	else {
	    hkReceiver.hkParameter.collectionIntervalTicks =
	            intervalSecondsToInterval(isDiagnostics, collectionInterval);
	}

	LocalPoolDataSetBase* dataSet = dynamic_cast<LocalPoolDataSetBase*>(
	            owner->getDataSetHandle(sid));
	if(dataSet != nullptr) {
		dataSet->setReportingEnabled(enableReporting);
		dataSet->setIsDiagnostic(isDiagnostics);
	}

	hkReceiver.intervalCounter = 1;

	hkReceiversMap.emplace(packetDestination, hkReceiver);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t LocalDataPoolManager::handleHousekeepingMessage(
		CommandMessage* message) {
    Command_t command = message->getCommand();
    sid_t sid = HousekeepingMessage::getSid(message);
    switch(command) {
    case(HousekeepingMessage::ENABLE_PERIODIC_DIAGNOSTICS_GENERATION):
    	return togglePeriodicGeneration(sid, true, true);
    case(HousekeepingMessage::DISABLE_PERIODIC_DIAGNOSTICS_GENERATION):
		return togglePeriodicGeneration(sid, false, true);
    case(HousekeepingMessage::ENABLE_PERIODIC_HK_REPORT_GENERATION):
		return togglePeriodicGeneration(sid, true, false);
    case(HousekeepingMessage::DISABLE_PERIODIC_HK_REPORT_GENERATION):
		return togglePeriodicGeneration(sid, false, false);

    case(HousekeepingMessage::REPORT_DIAGNOSTICS_REPORT_STRUCTURES):
    case(HousekeepingMessage::REPORT_HK_REPORT_STRUCTURES):
		return generateSetStructurePacket(sid);

    case(HousekeepingMessage::MODIFY_DIAGNOSTICS_REPORT_COLLECTION_INTERVAL):
    case(HousekeepingMessage::MODIFY_PARAMETER_REPORT_COLLECTION_INTERVAL): {
    	float newCollIntvl = 0;
    	HousekeepingMessage::getCollectionIntervalModificationCommand(message,
    			&newCollIntvl);
    	if(command == HousekeepingMessage::
    			MODIFY_DIAGNOSTICS_REPORT_COLLECTION_INTERVAL) {
    		return changeCollectionInterval(sid, newCollIntvl, true);
    	}
    	else {
    		return changeCollectionInterval(sid, newCollIntvl, false);
    	}
    }

    case(HousekeepingMessage::GENERATE_ONE_PARAMETER_REPORT):
    case(HousekeepingMessage::GENERATE_ONE_DIAGNOSTICS_REPORT): {
    	LocalPoolDataSetBase* dataSet = dynamic_cast<LocalPoolDataSetBase*>(
    			owner->getDataSetHandle(sid));
    	if(command == HousekeepingMessage::GENERATE_ONE_PARAMETER_REPORT
    			and dataSet->getIsDiagnostics()) {
    		return WRONG_HK_PACKET_TYPE;
    	}
    	else if(command == HousekeepingMessage::GENERATE_ONE_DIAGNOSTICS_REPORT
    			and not dataSet->getIsDiagnostics()) {
    		return WRONG_HK_PACKET_TYPE;
    	}
    	return generateHousekeepingPacket(HousekeepingMessage::getSid(message),
    			dataSet, true);
    }

    default:
        return CommandMessageIF::UNKNOWN_COMMAND;
    }
}

ReturnValue_t LocalDataPoolManager::printPoolEntry(
		lp_id_t localPoolId) {
	auto poolIter = localPoolMap.find(localPoolId);
	if (poolIter == localPoolMap.end()) {
		sif::debug << "HousekeepingManager::fechPoolEntry:"
				" Pool entry not found." << std::endl;
		return POOL_ENTRY_NOT_FOUND;
	}
	poolIter->second->print();
	return HasReturnvaluesIF::RETURN_OK;
}

MutexIF* LocalDataPoolManager::getMutexHandle() {
	return mutex;
}

const HasLocalDataPoolIF* LocalDataPoolManager::getOwner() const {
    return owner;
}

ReturnValue_t LocalDataPoolManager::generateHousekeepingPacket(sid_t sid,
		LocalPoolDataSetBase* dataSet, bool forDownlink,
		MessageQueueId_t destination) {
	if(dataSet == nullptr) {
		// Configuration error.
		sif::warning << "HousekeepingManager::generateHousekeepingPacket:"
				" Set ID not found or dataset not assigned!" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	store_address_t storeId;
	HousekeepingPacketDownlink hkPacket(sid, dataSet);
	size_t serializedSize = 0;
	ReturnValue_t result = serializeHkPacketIntoStore(hkPacket, storeId,
			forDownlink, &serializedSize);
	if(result != HasReturnvaluesIF::RETURN_OK or serializedSize == 0) {
	    return result;
	}

	// and now we set a HK message and send it the HK packet destination.
	CommandMessage hkMessage;
	if(dataSet->getIsDiagnostics()) {
		HousekeepingMessage::setHkDiagnosticsReply(&hkMessage, sid, storeId);
	}
	else {
		HousekeepingMessage::setHkReportReply(&hkMessage, sid, storeId);
	}

	if(hkQueue == nullptr) {
	    return QUEUE_OR_DESTINATION_NOT_SET;
	}
	if(destination == MessageQueueIF::NO_QUEUE) {
	    if(hkDestinationId == MessageQueueIF::NO_QUEUE) {
	        // error, all destinations invalid
	        return HasReturnvaluesIF::RETURN_FAILED;
	    }
	    destination = hkDestinationId;
	}

	return hkQueue->sendMessage(destination, &hkMessage);
}

ReturnValue_t LocalDataPoolManager::serializeHkPacketIntoStore(
        HousekeepingPacketDownlink& hkPacket,
        store_address_t& storeId, bool forDownlink,
		size_t* serializedSize) {
    uint8_t* dataPtr = nullptr;
    const size_t maxSize = hkPacket.getSerializedSize();
    ReturnValue_t result = ipcStore->getFreeElement(&storeId,
            maxSize, &dataPtr);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    if(forDownlink) {
    	return hkPacket.serialize(&dataPtr, serializedSize, maxSize,
    	            SerializeIF::Endianness::BIG);
    }
    return hkPacket.serialize(&dataPtr, serializedSize, maxSize,
            SerializeIF::Endianness::MACHINE);
}

ReturnValue_t LocalDataPoolManager::generateSetStructurePacket(sid_t sid) {
    LocalPoolDataSetBase* dataSet = dynamic_cast<LocalPoolDataSetBase*>(
            owner->getDataSetHandle(sid));
    if(dataSet == nullptr) {
        sif::warning << "HousekeepingManager::generateHousekeepingPacket:"
                " Set ID not found" << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    size_t expectedSize = dataSet->getFillCount() * sizeof(lp_id_t);
    uint8_t* storePtr = nullptr;
    store_address_t storeId;
    ReturnValue_t result = ipcStore->getFreeElement(&storeId,
            expectedSize,&storePtr);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "HousekeepingManager::generateHousekeepingPacket: "
                "Could not get free element from IPC store." << std::endl;
        return result;
    }
    size_t size = 0;
    result = dataSet->serializeLocalPoolIds(&storePtr, &size,
            expectedSize, SerializeIF::Endianness::BIG);
    if(expectedSize != size) {
        sif::error << "HousekeepingManager::generateSetStructurePacket: "
                "Expected size is not equal to serialized size" << std::endl;
    }
    return result;
}

void LocalDataPoolManager::setNonDiagnosticIntervalFactor(
        uint8_t nonDiagInvlFactor) {
    this->nonDiagnosticIntervalFactor = nonDiagInvlFactor;
}


void LocalDataPoolManager::performPeriodicHkGeneration(HkReceiver* receiver) {
    if(receiver->intervalCounter >=
            receiver->hkParameter.collectionIntervalTicks) {
    	sid_t sid = receiver->dataId.dataSetSid;
    	LocalPoolDataSetBase* dataSet = dynamic_cast<LocalPoolDataSetBase*>(
    			owner->getDataSetHandle(sid));
    	if(not dataSet->getReportingEnabled()) {
    		return;
    	}

        ReturnValue_t result = generateHousekeepingPacket(
                sid, dataSet, true);
        if(result != HasReturnvaluesIF::RETURN_OK) {
            // configuration error
            sif::debug << "LocalDataPoolManager::performHkOperation:"
                    << "0x" << std::setfill('0') << std::setw(8)
                    << owner->getObjectId() << " Error generating "
                    << "HK packet" << std::setfill(' ') << std::endl;
        }
        receiver->intervalCounter = 1;
    }
    else {
        receiver->intervalCounter++;
    }
}

uint32_t LocalDataPoolManager::intervalSecondsToInterval(bool isDiagnostics,
        float collectionIntervalSeconds) {
    if(isDiagnostics) {
        return std::ceil(collectionIntervalSeconds * 1000
                /diagnosticMinimumInterval);
    }
    else {
        return std::ceil(collectionIntervalSeconds * 1000
                /regularMinimumInterval);
    }
}

float LocalDataPoolManager::intervalToIntervalSeconds(bool isDiagnostics,
        uint32_t collectionInterval) {
    if(isDiagnostics) {
        return static_cast<float>(collectionInterval *
        		diagnosticMinimumInterval);
    }
    else {
        return static_cast<float>(collectionInterval *
                regularMinimumInterval);
    }
}

ReturnValue_t LocalDataPoolManager::togglePeriodicGeneration(sid_t sid,
		bool enable, bool isDiagnostics) {
	LocalPoolDataSetBase* dataSet = dynamic_cast<LocalPoolDataSetBase*>(
			owner->getDataSetHandle(sid));
	if((dataSet->getIsDiagnostics() and not isDiagnostics) or
			(not dataSet->getIsDiagnostics() and isDiagnostics)) {
		return WRONG_HK_PACKET_TYPE;
	}

	if((dataSet->getReportingEnabled() and enable) or
			(not dataSet->getReportingEnabled() and not enable)) {
		return REPORTING_STATUS_UNCHANGED;
	}

	dataSet->setReportingEnabled(enable);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t LocalDataPoolManager::changeCollectionInterval(sid_t sid,
		float newCollectionInterval, bool isDiagnostics) {
	LocalPoolDataSetBase* dataSet = dynamic_cast<LocalPoolDataSetBase*>(
				owner->getDataSetHandle(sid));
	bool targetIsDiagnostics = dataSet->getIsDiagnostics();
	if((targetIsDiagnostics and not isDiagnostics) or
			(not targetIsDiagnostics and isDiagnostics)) {
		return WRONG_HK_PACKET_TYPE;
	}

	for(auto& receiver: hkReceiversMap) {
		if(receiver.second.reportingType != ReportingType::PERIODIC) {
			continue;
		}

		uint32_t newInterval = intervalSecondsToInterval(isDiagnostics,
				newCollectionInterval);
		receiver.second.hkParameter.collectionIntervalTicks = newInterval;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

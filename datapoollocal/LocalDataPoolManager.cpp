#include "../datapoollocal/LocalDataPoolManager.h"
#include "../datapoollocal/LocalDataSet.h"
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
}

ReturnValue_t LocalDataPoolManager::initialize(MessageQueueIF* queueToUse,
        uint8_t nonDiagInvlFactor) {
    if(queueToUse == nullptr) {
        sif::error << "LocalDataPoolManager::initialize: Supplied queue "
                "invalid!" << std::endl;
    }
    hkQueue = queueToUse;

    setNonDiagnosticIntervalFactor(nonDiagInvlFactor);
    diagnosticMinimumInterval = owner->getPeriodicOperationFrequency();
    regularMinimumInterval = diagnosticMinimumInterval * nonDiagnosticIntervalFactor;
    return initializeHousekeepingPoolEntriesOnce();
}

void LocalDataPoolManager::setHkPacketDestination(
        MessageQueueId_t hkDestination) {
    this->hkDestination = hkDestination;
}

LocalDataPoolManager::~LocalDataPoolManager() {}

ReturnValue_t LocalDataPoolManager::initializeHousekeepingPoolEntriesOnce() {
	if(not mapInitialized) {
		ReturnValue_t result = owner->initializePoolEntries(localPoolMap);
		if(result == HasReturnvaluesIF::RETURN_OK) {
			mapInitialized = true;
		}
		return result;
	}
	sif::warning << "HousekeepingManager: The map should only be initialized "
	        "once!" << std::endl;
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t LocalDataPoolManager::handleHousekeepingMessage(
		CommandMessage* message) {
    Command_t command = message->getCommand();
    switch(command) {
    // I think those are the only commands which can be handled here..
    case(HousekeepingMessage::ADD_HK_REPORT_STRUCT):
    case(HousekeepingMessage::ADD_DIAGNOSTICS_REPORT_STRUCT):
        // We should use OwnsLocalPoolDataIF to specify those functions..
        return HasReturnvaluesIF::RETURN_OK;
    case(HousekeepingMessage::REPORT_DIAGNOSTICS_REPORT_STRUCTURES):
    case(HousekeepingMessage::REPORT_HK_REPORT_STRUCTURES):
        //return generateSetStructurePacket(message->getSid());
    case(HousekeepingMessage::GENERATE_ONE_PARAMETER_REPORT):
    case(HousekeepingMessage::GENERATE_ONE_DIAGNOSTICS_REPORT):
        //return generateHousekeepingPacket(message->getSid());
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
        float collectionInterval, MessageQueueId_t sendTo) {
	LocalPoolDataSetBase* dataSetToSerialize = dynamic_cast<LocalPoolDataSetBase*>(
			owner->getDataSetHandle(sid));
	if(dataSetToSerialize == nullptr) {
		sif::warning << "HousekeepingManager::generateHousekeepingPacket:"
				" Set ID not found" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	store_address_t storeId;
	HousekeepingPacketDownlink hkPacket(sid, collectionInterval,
	        dataSetToSerialize->getFillCount(), dataSetToSerialize);
	ReturnValue_t result = serializeHkPacketIntoStore(hkPacket, &storeId);
	if(result != HasReturnvaluesIF::RETURN_OK) {
	    return result;
	}

	// and now we set a HK message and send it the HK packet destination.
	CommandMessage hkMessage;
	HousekeepingMessage::setHkReportMessage(&hkMessage, sid, storeId);
	if(hkQueue == nullptr) {
	    return QUEUE_OR_DESTINATION_NOT_SET;
	}

	if(sendTo != MessageQueueIF::NO_QUEUE) {
	    result = hkQueue->sendMessage(sendTo, &hkMessage);
	}
	else {
	    if(hkDestination == MessageQueueIF::NO_QUEUE) {
	        sif::warning << "LocalDataPoolManager::generateHousekeepingPacket:"
	                " Destination is not set properly!" << std::endl;
	        return QUEUE_OR_DESTINATION_NOT_SET;
	    }
	    else {
	        result = hkQueue->sendMessage(hkDestination, &hkMessage);
	    }
	}

	return result;
}

ReturnValue_t LocalDataPoolManager::serializeHkPacketIntoStore(
        HousekeepingPacketDownlink& hkPacket,
        store_address_t *storeId) {
    uint8_t* dataPtr = nullptr;
    size_t serializedSize = hkPacket.getSerializedSize();
    const size_t maxSize = serializedSize;
    ReturnValue_t  result = ipcStore->getFreeElement(storeId,
            serializedSize, &dataPtr);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    return hkPacket.serialize(&dataPtr, &serializedSize, maxSize,
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


ReturnValue_t LocalDataPoolManager::performHkOperation() {
    for(auto& hkReceiversIter: hkReceiversMap) {
        HkReceiver* receiver = &hkReceiversIter.second;
        if(not receiver->reportingEnabled) {
            return HasReturnvaluesIF::RETURN_OK;
        }

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

void LocalDataPoolManager::performPeriodicHkGeneration(HkReceiver* receiver) {
    if(receiver->intervalCounter >= intervalSecondsToInterval(
            receiver->isDiagnostics,
            receiver->hkParameter.collectionIntervalSeconds)) {
        ReturnValue_t result = generateHousekeepingPacket(
                receiver->dataId.dataSetSid, receiver->destinationQueue);
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
        return std::ceil(collectionIntervalSeconds/diagnosticMinimumInterval);
    }
    else {
        return std::ceil(collectionIntervalSeconds/regularMinimumInterval);
    }
}

float LocalDataPoolManager::intervalToIntervalSeconds(bool isDiagnostics,
        uint32_t collectionInterval) {
    if(isDiagnostics) {
        return static_cast<float>(collectionInterval * diagnosticMinimumInterval);
    }
    else {
        return static_cast<float>(collectionInterval * regularMinimumInterval);
    }
}

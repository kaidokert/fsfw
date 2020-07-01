#include <framework/datapoollocal/LocalDataPoolManager.h>
#include <framework/datapoollocal/LocalDataSet.h>
#include <framework/housekeeping/AcceptsHkPacketsIF.h>
#include <framework/ipc/MutexFactory.h>
#include <framework/ipc/MutexHelper.h>
#include <framework/ipc/QueueFactory.h>

#include <array>

LocalDataPoolManager::LocalDataPoolManager(OwnsLocalDataPoolIF* owner,
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
        object_id_t hkDestination) {
    if(queueToUse == nullptr) {
        sif::error << "LocalDataPoolManager::initialize: Supplied queue "
                "invalid!" << std::endl;
    }
    hkQueue = queueToUse;

    if(hkDestination == objects::NO_OBJECT) {
        return initializeHousekeepingPoolEntriesOnce();
    }

    AcceptsHkPacketsIF* hkReceiver =
            objectManager->get<AcceptsHkPacketsIF>(hkDestination);
    if(hkReceiver != nullptr) {
        setHkPacketDestination(hkReceiver->getHkQueue());
    }
    else {
        sif::warning << "LocalDataPoolManager::initialize: Could not retrieve"
                " queue ID from HK destination object ID. Make sure it exists"
                " and the object impements AcceptsHkPacketsIF!" << std::endl;
    }
    return initializeHousekeepingPoolEntriesOnce();
}

void LocalDataPoolManager::setHkPacketDestination(
        MessageQueueId_t hkDestination) {
    this->hkDestination = hkDestination;
}

LocalDataPoolManager::~LocalDataPoolManager() {}

ReturnValue_t LocalDataPoolManager::initializeHousekeepingPoolEntriesOnce() {
	if(not mapInitialized) {
		ReturnValue_t result = owner->initializePoolEntries(localDpMap);
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
	auto poolIter = localDpMap.find(localPoolId);
	if (poolIter == localDpMap.end()) {
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

const OwnsLocalDataPoolIF* LocalDataPoolManager::getOwner() const {
    return owner;
}

ReturnValue_t LocalDataPoolManager::generateHousekeepingPacket(sid_t sid,
        MessageQueueId_t sendTo) {
	LocalDataSet* dataSetToSerialize = dynamic_cast<LocalDataSet*>(
			owner->getDataSetHandle(sid));
	if(dataSetToSerialize == nullptr) {
		sif::warning << "HousekeepingManager::generateHousekeepingPacket:"
				" Set ID not found" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	store_address_t storeId;
	ReturnValue_t result = serializeHkPacketIntoStore(&storeId,
	        dataSetToSerialize);
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

ReturnValue_t LocalDataPoolManager::generateSetStructurePacket(sid_t sid) {
    LocalDataSet* dataSet = dynamic_cast<LocalDataSet*>(
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

void LocalDataPoolManager::setMinimalSamplingFrequency(float frequencySeconds) {
}

ReturnValue_t LocalDataPoolManager::serializeHkPacketIntoStore(
        store_address_t *storeId, LocalDataSet* dataSet) {
    size_t hkSize = dataSet->getSerializedSize();
    uint8_t* storePtr = nullptr;
    ReturnValue_t result = ipcStore->getFreeElement(storeId, hkSize,&storePtr);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "HousekeepingManager::generateHousekeepingPacket: "
                "Could not get free element from IPC store." << std::endl;
        return result;
    }
    size_t size = 0;

    if(appendValidityBuffer) {
        result = dataSet->serializeWithValidityBuffer(&storePtr,
                &size, hkSize, SerializeIF::Endianness::MACHINE);
    }
    else {
        result = dataSet->serialize(&storePtr, &size, hkSize,
                SerializeIF::Endianness::MACHINE);
    }

    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "HousekeepingManager::serializeHkPacketIntoStore: "
                "Serialization proccess failed!" << std::endl;
    }
    return result;
}



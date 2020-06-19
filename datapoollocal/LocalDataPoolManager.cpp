#include <framework/datapoollocal/LocalDataPoolManager.h>
#include <framework/datapoollocal/LocalDataSet.h>
#include <framework/ipc/MutexFactory.h>
#include <framework/ipc/MutexHelper.h>
#include <framework/ipc/QueueFactory.h>

#include <array>

LocalDataPoolManager::LocalDataPoolManager(OwnsLocalDataPoolIF* owner,
        uint32_t replyQueueDepth, bool appendValidityBuffer):
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
	hkQueue = QueueFactory::instance()->createMessageQueue(replyQueueDepth,
	        HousekeepingMessage::HK_MESSAGE_SIZE);
}

LocalDataPoolManager::~LocalDataPoolManager() {}

ReturnValue_t LocalDataPoolManager::initializeHousekeepingPoolEntriesOnce() {
	if(not mapInitialized) {
		ReturnValue_t result =
				owner->initializePoolEntries(localDpMap);
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
		HousekeepingMessage& message) {
    Command_t command = message.getCommand();
    switch(command) {
    // I think those are the only commands which can be handled here..
    case(HousekeepingMessage::ADD_HK_REPORT_STRUCT):
    case(HousekeepingMessage::ADD_DIAGNOSTICS_REPORT_STRUCT):
        // We should use OwnsLocalPoolDataIF to specify those functions..
        return HasReturnvaluesIF::RETURN_OK;
    case(HousekeepingMessage::REPORT_DIAGNOSTICS_REPORT_STRUCTURES):
    case(HousekeepingMessage::REPORT_HK_REPORT_STRUCTURES):
        return generateSetStructurePacket(message.getSid());
    case(HousekeepingMessage::GENERATE_ONE_PARAMETER_REPORT):
    case(HousekeepingMessage::GENERATE_ONE_DIAGNOSTICS_REPORT):
        return generateHousekeepingPacket(message.getSid());
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

void LocalDataPoolManager::setHkPacketDestination(
        MessageQueueId_t destinationQueueId) {
    this->currentHkPacketDestination = destinationQueueId;
}

const OwnsLocalDataPoolIF* LocalDataPoolManager::getOwner() const {
    return owner;
}

ReturnValue_t LocalDataPoolManager::generateHousekeepingPacket(sid_t sid) {
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
	MessageQueueMessage message;
	HousekeepingMessage hkMessage(&message);
	hkMessage.setHkReportMessage(sid, storeId);
	if(hkQueue == nullptr) {
	    return QUEUE_NOT_SET;
	}

	if(currentHkPacketDestination != MessageQueueIF::NO_QUEUE) {
	    result = hkQueue->sendMessage(currentHkPacketDestination, &hkMessage);
	}
	else {
	    result = hkQueue->sendToDefault(&hkMessage);
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
            expectedSize, false);
    if(expectedSize != size) {
        sif::error << "HousekeepingManager::generateSetStructurePacket: "
                "Expected size is not equal to serialized size" << std::endl;
    }
    return result;
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
                &size, hkSize, false);
    }
    else {
        result = dataSet->serialize(&storePtr, &size, hkSize, false);
    }

    if(result != HasReturnvaluesIF::RETURN_OK) {
        sif::error << "HousekeepingManager::serializeHkPacketIntoStore: "
                "Serialization proccess failed!" << std::endl;
    }
    return result;
}




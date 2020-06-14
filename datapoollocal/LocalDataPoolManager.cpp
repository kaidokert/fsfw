#include <framework/datapoollocal/LocalDataPoolManager.h>
#include <framework/datapoollocal/LocalDataSet.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/objectmanager/frameworkObjects.h>
#include <framework/ipc/MutexFactory.h>
#include <framework/ipc/MutexHelper.h>

#include <array>

LocalDataPoolManager::LocalDataPoolManager(OwnsLocalDataPoolIF* owner) {
	if(owner == nullptr) {
		sif::error << "HkManager: Invalid supplied owner!" << std::endl;
		std::exit(0);
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
}

LocalDataPoolManager::~LocalDataPoolManager() {}

ReturnValue_t LocalDataPoolManager::initializeHousekeepingPoolEntriesOnce() {
	if(not mapInitialized) {
		ReturnValue_t result =
				owner->initializeHousekeepingPoolEntries(localDpMap);
		if(result == HasReturnvaluesIF::RETURN_OK) {
			mapInitialized = true;
		}
		return result;
	}
	sif::warning << "HousekeepingManager: The map" << std::endl;
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t LocalDataPoolManager::handleHousekeepingMessage(
		HousekeepingMessage& message) {
	return HasReturnvaluesIF::RETURN_FAILED;
}

ReturnValue_t LocalDataPoolManager::printPoolEntry(
		lp_id_t localPoolId) {
	auto poolIter = localDpMap.find(localPoolId);
	if (poolIter == localDpMap.end()) {
		sif::debug << "HousekeepingManager::fechPoolEntry:"
				" Pool entry not found." << std::endl;
		return OwnsLocalDataPoolIF::POOL_ENTRY_NOT_FOUND;
	}
	poolIter->second->print();
	return HasReturnvaluesIF::RETURN_OK;
}

MutexIF* LocalDataPoolManager::getMutexHandle() {
	return mutex;
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
	size_t hkSize = dataSetToSerialize->getSerializedSize();
	uint8_t* storePtr = nullptr;
	ReturnValue_t result = ipcStore->getFreeElement(&storeId, hkSize,&storePtr);
	if(result != HasReturnvaluesIF::RETURN_OK) {
	    sif::warning << "HousekeepingManager::generateHousekeepingPacket: "
	            "Could not get free element from IPC store." << std::endl;
	    return result;
	}
	size_t size = 0;
	dataSetToSerialize->serialize(&storePtr, &size, hkSize, false);
	// and now we have to set a HK message and send it the queue.
	return HasReturnvaluesIF::RETURN_OK;
}

void LocalDataPoolManager::setHkPacketQueue(MessageQueueIF *msgQueue) {
	this->hkPacketQueue = msgQueue;
}

void LocalDataPoolManager::setHkReplyQueue(MessageQueueIF *replyQueue) {
    this->hkReplyQueue = replyQueue;
}

const OwnsLocalDataPoolIF* LocalDataPoolManager::getOwner() const {
	return owner;
}

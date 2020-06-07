#include <framework/datapoollocal/LocalDataPoolManager.h>
#include <framework/datapoollocal/LocalDataSet.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
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
	//owner->setMinimalHkSamplingFrequency();
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
		CommandMessage *message) {
	return HasReturnvaluesIF::RETURN_OK;
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

//void HousekeepingManager::setMinimalSamplingFrequency(float frequencySeconds) {
//	this->samplingFrequency = frequencySeconds;
//
//}

void LocalDataPoolManager::generateHousekeepingPacket(sid_t sid) {
	LocalDataSet* dataSetToSerialize = dynamic_cast<LocalDataSet*>(
			owner->getDataSetHandle(sid));
	if(dataSetToSerialize == nullptr) {
		sif::warning << "HousekeepingManager::generateHousekeepingPacket:"
				" Set ID not found" << std::endl;
		return;
	}
	std::array<uint8_t, 256> testBuffer = {};
	uint8_t* dataPtr = testBuffer.data();
	size_t size = 0;
	dataSetToSerialize->serialize(&dataPtr, &size, testBuffer.size(),
			false);
	// and now we send it to the TM funnel or somewhere else

}

void LocalDataPoolManager::setHkPacketQueue(MessageQueueIF *msgQueue) {
	this->hkPacketQueue = msgQueue;
}

const OwnsLocalDataPoolIF* LocalDataPoolManager::getOwner() const {
	return owner;
}

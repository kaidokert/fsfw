#include <framework/datapoollocal/LocalDataSet.h>
#include <framework/housekeeping/HousekeepingManager.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/ipc/MutexFactory.h>
#include <framework/ipc/MutexHelper.h>

#include <array>

HousekeepingManager::HousekeepingManager(HasHkPoolParametersIF* owner) {
	if(owner == nullptr) {
		sif::error << "HkManager: Invalid supplied owner!" << std::endl;
		std::exit(0);
	}
	this->owner = owner;
	mutex = MutexFactory::instance()->createMutex();
	//owner->setMinimalHkSamplingFrequency();
}

HousekeepingManager::~HousekeepingManager() {}

ReturnValue_t HousekeepingManager::initializeHousekeepingPoolEntriesOnce() {
	if(not mapInitialized) {
		ReturnValue_t result = owner->initializeHousekeepingPoolEntries(localDpMap);
		if(result == HasReturnvaluesIF::RETURN_OK) {
			mapInitialized = true;
		}
		return result;
	}
	sif::warning << "HousekeepingManager: The map" << std::endl;
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t HousekeepingManager::handleHousekeepingMessage(
		CommandMessage *message) {
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t HousekeepingManager::printPoolEntry(
		lp_id_t localPoolId) {
	auto poolIter = localDpMap.find(localPoolId);
	if (poolIter == localDpMap.end()) {
		sif::debug << "HousekeepingManager::fechPoolEntry:"
				" Pool entry not found." << std::endl;
		return HasHkPoolParametersIF::POOL_ENTRY_NOT_FOUND;
	}
	poolIter->second->print();
	return HasReturnvaluesIF::RETURN_OK;
}

MutexIF* HousekeepingManager::getMutexHandle() {
	return mutex;
}

//void HousekeepingManager::setMinimalSamplingFrequency(float frequencySeconds) {
//	this->samplingFrequency = frequencySeconds;
//
//}

void HousekeepingManager::generateHousekeepingPacket(sid_t sid) {
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

void HousekeepingManager::setHkPacketQueue(MessageQueueIF *msgQueue) {
	this->hkPacketQueue = msgQueue;
}

const HasHkPoolParametersIF* HousekeepingManager::getOwner() const {
	return owner;
}

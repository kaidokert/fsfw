#include <framework/housekeeping/HousekeepingManager.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/ipc/MutexFactory.h>
#include <framework/ipc/MutexHelper.h>

HousekeepingManager::HousekeepingManager(HasHkPoolParametersIF* owner) {
	//todo :: nullptr check owner.
	if(owner == nullptr) {
		sif::error << "HkManager: Invalid supplied owner!" << std::endl;
		std::exit(0);
	}
	this->owner = owner;
	mutex = MutexFactory::instance()->createMutex();
	owner->setMinimalHkSamplingFrequency();
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
	sif::warning << "hk manager says no" << std::endl;
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t HousekeepingManager::handleHousekeepingMessage(
		CommandMessage *message) {
	return HasReturnvaluesIF::RETURN_OK;
}

MutexIF* HousekeepingManager::getMutexHandle() {
	return mutex;
}

void HousekeepingManager::setMinimalSamplingFrequency(float frequencySeconds) {
	this->samplingFrequency = frequencySeconds;

}

void HousekeepingManager::generateHousekeepingPacket(DataSetIF *dataSet) {
}

void HousekeepingManager::setHkPacketQueue(MessageQueueIF *msgQueue) {
	this->hkPacketQueue = msgQueue;
}

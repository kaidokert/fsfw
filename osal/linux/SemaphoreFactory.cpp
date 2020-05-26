#include <framework/tasks/SemaphoreFactory.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

const uint32_t SemaphoreIF::NO_TIMEOUT = 0;

SemaphoreFactory* SemaphoreFactory::factoryInstance = nullptr;

SemaphoreFactory::SemaphoreFactory() {
}

SemaphoreFactory::~SemaphoreFactory() {
	delete factoryInstance;
}

SemaphoreFactory* SemaphoreFactory::instance() {
	if (factoryInstance == nullptr){
		factoryInstance = new SemaphoreFactory();
	}
	return SemaphoreFactory::factoryInstance;
}

SemaphoreIF* SemaphoreFactory::createBinarySemaphore() {
	sif::error << "Semaphore not implemented for Linux yet" << std::endl;
	return nullptr;
}

SemaphoreIF* SemaphoreFactory::createCountingSemaphore(uint8_t count,
		uint8_t initCount) {
	sif::error << "Counting Semaphore not implemented for "
			"Linux yet" << std::endl;
	return nullptr;
}

void SemaphoreFactory::deleteMutex(SemaphoreIF* semaphore) {
	delete semaphore;
}

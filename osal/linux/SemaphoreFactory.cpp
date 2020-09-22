#include "../../tasks/SemaphoreFactory.h"
#include "BinarySemaphore.h"
#include "CountingSemaphore.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"

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

SemaphoreIF* SemaphoreFactory::createBinarySemaphore(uint32_t arguments) {
	return new BinarySemaphore();
}

SemaphoreIF* SemaphoreFactory::createCountingSemaphore(const uint8_t maxCount,
		uint8_t initCount, uint32_t arguments) {
	return new CountingSemaphore(maxCount, initCount);
}

void SemaphoreFactory::deleteSemaphore(SemaphoreIF* semaphore) {
	delete semaphore;
}

#include <framework/osal/FreeRTOS/BinarySemaphore.h>
#include <framework/osal/FreeRTOS/CountingSemaphore.h>
#include <framework/tasks/SemaphoreFactory.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

SemaphoreFactory* SemaphoreFactory::factoryInstance = nullptr;
const uint32_t SemaphoreIF::NO_TIMEOUT = 0;
const uint32_t SemaphoreIF::MAX_TIMEOUT = portMAX_DELAY;

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

SemaphoreIF* SemaphoreFactory::createBinarySemaphore(uint32_t argument) {
	return new BinarySemaphore();
}

SemaphoreIF* SemaphoreFactory::createCountingSemaphore(uint8_t count,
		uint8_t initCount, uint32_t argument) {
	return new CountingSemaphore(count, initCount);
}

void SemaphoreFactory::deleteSemaphore(SemaphoreIF* semaphore) {
	delete semaphore;
}

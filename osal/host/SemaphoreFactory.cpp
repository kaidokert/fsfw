#include "../../tasks/SemaphoreFactory.h"
#include "../../osal/linux/BinarySemaphore.h"
#include "../../osal/linux/CountingSemaphore.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"

const uint32_t SemaphoreIF::POLLING = 0;
const uint32_t SemaphoreIF::BLOCKING = 0xFFFFFFFF;

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
	// Just gonna wait for full C++20 for now.
	sif::error << "SemaphoreFactory: Binary Semaphore not implemented yet."
			" Returning nullptr!\n" << std::flush;
	return nullptr;
}

SemaphoreIF* SemaphoreFactory::createCountingSemaphore(const uint8_t maxCount,
		uint8_t initCount, uint32_t arguments) {
	// Just gonna wait for full C++20 for now.
	sif::error << "SemaphoreFactory: Counting Semaphore not implemented yet."
				" Returning nullptr!\n" << std::flush;
	return nullptr;
}

void SemaphoreFactory::deleteSemaphore(SemaphoreIF* semaphore) {
	delete semaphore;
}

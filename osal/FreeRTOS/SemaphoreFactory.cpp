#include "../../osal/FreeRTOS/BinarySemaphore.h"
#include "../../osal/FreeRTOS/BinSemaphUsingTask.h"
#include "../../osal/FreeRTOS/CountingSemaphore.h"
#include "../../osal/FreeRTOS/CountingSemaphUsingTask.h"
#include "../../tasks/SemaphoreFactory.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"

SemaphoreFactory* SemaphoreFactory::factoryInstance = nullptr;

static const uint32_t USE_REGULAR_SEMAPHORES = 0;
static const uint32_t USE_TASK_NOTIFICATIONS = 1;

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
	if(argument == USE_REGULAR_SEMAPHORES) {
		return new BinarySemaphore();
	}
	else if(argument == USE_TASK_NOTIFICATIONS) {
		return new BinarySemaphoreUsingTask();
	}
	else {
		sif::warning << "SemaphoreFactory: Invalid argument, return regular"
				"binary semaphore" << std::endl;
		return new BinarySemaphore();
	}
}

SemaphoreIF* SemaphoreFactory::createCountingSemaphore(uint8_t maxCount,
		uint8_t initCount, uint32_t argument) {
	if(argument == USE_REGULAR_SEMAPHORES) {
		return new CountingSemaphore(maxCount, initCount);
	}
	else if(argument == USE_TASK_NOTIFICATIONS) {
		return new CountingSemaphoreUsingTask(maxCount, initCount);
	}
	else {
		sif::warning << "SemaphoreFactory: Invalid argument, return regular"
						"binary semaphore" << std::endl;
		return new CountingSemaphore(maxCount, initCount);
	}

}

void SemaphoreFactory::deleteSemaphore(SemaphoreIF* semaphore) {
	delete semaphore;
}

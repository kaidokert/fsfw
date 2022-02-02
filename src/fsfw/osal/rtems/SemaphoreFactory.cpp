#include "fsfw/osal/rtems/BinarySemaphore.h"
//#include "fsfw/osal/rtems/CountingSemaphore.h"

#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tasks/SemaphoreFactory.h"

SemaphoreFactory* SemaphoreFactory::factoryInstance = nullptr;

SemaphoreFactory::SemaphoreFactory() {}

SemaphoreFactory::~SemaphoreFactory() { delete factoryInstance; }

SemaphoreFactory* SemaphoreFactory::instance() {
  if (factoryInstance == nullptr) {
    factoryInstance = new SemaphoreFactory();
  }
  return SemaphoreFactory::factoryInstance;
}

SemaphoreIF* SemaphoreFactory::createBinarySemaphore(uint32_t argument) {
  return new BinarySemaphore();
}

SemaphoreIF* SemaphoreFactory::createCountingSemaphore(uint8_t maxCount, uint8_t initCount,
                                                       uint32_t argument) {
  return nullptr;
}

void SemaphoreFactory::deleteSemaphore(SemaphoreIF* semaphore) { delete semaphore; }

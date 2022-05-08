#include "fsfw/tasks/SemaphoreFactory.h"

#include "fsfw/serviceinterface.h"

SemaphoreFactory* SemaphoreFactory::factoryInstance = nullptr;

SemaphoreFactory::SemaphoreFactory() {}

SemaphoreFactory::~SemaphoreFactory() { delete factoryInstance; }

SemaphoreFactory* SemaphoreFactory::instance() {
  if (factoryInstance == nullptr) {
    factoryInstance = new SemaphoreFactory();
  }
  return SemaphoreFactory::factoryInstance;
}

SemaphoreIF* SemaphoreFactory::createBinarySemaphore(uint32_t arguments) {
  // Just gonna wait for full C++20 for now.
  FSFW_LOGE("SemaphoreFactory: Binary Semaphore not implemented yet. Returning nullptr!\n");
  return nullptr;
}

SemaphoreIF* SemaphoreFactory::createCountingSemaphore(const uint8_t maxCount, uint8_t initCount,
                                                       uint32_t arguments) {
  // Just gonna wait for full C++20 for now.
  FSFW_LOGE("SemaphoreFactory: Counting Semaphore not implemented yet. Returning nullptr!\n");
  return nullptr;
}

void SemaphoreFactory::deleteSemaphore(SemaphoreIF* semaphore) { delete semaphore; }

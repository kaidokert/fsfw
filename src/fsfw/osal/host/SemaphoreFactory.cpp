#include "fsfw/tasks/SemaphoreFactory.h"

#include "fsfw/serviceinterface/ServiceInterface.h"

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
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::error << "SemaphoreFactory: Binary Semaphore not implemented yet."
                " Returning nullptr!\n"
             << std::flush;
#endif
  return nullptr;
}

SemaphoreIF* SemaphoreFactory::createCountingSemaphore(const uint8_t maxCount, uint8_t initCount,
                                                       uint32_t arguments) {
  // Just gonna wait for full C++20 for now.
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::error << "SemaphoreFactory: Counting Semaphore not implemented yet."
                " Returning nullptr!\n"
             << std::flush;
#endif
  return nullptr;
}

void SemaphoreFactory::deleteSemaphore(SemaphoreIF* semaphore) { delete semaphore; }

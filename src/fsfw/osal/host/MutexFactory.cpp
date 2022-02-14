#include "fsfw/ipc/MutexFactory.h"

#include "fsfw/osal/host/Mutex.h"

// TODO: Different variant than the lazy loading in QueueFactory.
// What's better and why? -> one is on heap the other on bss/data
// MutexFactory* MutexFactory::factoryInstance = new MutexFactory();
MutexFactory* MutexFactory::factoryInstance = nullptr;

MutexFactory::MutexFactory() {}

MutexFactory::~MutexFactory() {}

MutexFactory* MutexFactory::instance() {
  if (factoryInstance == nullptr) {
    factoryInstance = new MutexFactory();
  }
  return MutexFactory::factoryInstance;
}

MutexIF* MutexFactory::createMutex() { return new Mutex(); }

void MutexFactory::deleteMutex(MutexIF* mutex) {
  if (mutex != nullptr) {
    delete mutex;
  }
}

#include "fsfw/ipc/MutexFactory.h"

#include "fsfw/osal/rtems/Mutex.h"

MutexFactory* MutexFactory::factoryInstance = new MutexFactory();

MutexFactory::MutexFactory() {}

MutexFactory::~MutexFactory() {}

MutexFactory* MutexFactory::instance() { return MutexFactory::factoryInstance; }

MutexIF* MutexFactory::createMutex() { return new Mutex(); }

void MutexFactory::deleteMutex(MutexIF* mutex) { delete mutex; }

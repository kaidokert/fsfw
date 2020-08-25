#include "../../ipc/MutexFactory.h"
#include "Mutex.h"

//TODO: Different variant than the lazy loading in QueueFactory. What's better and why?
MutexFactory* MutexFactory::factoryInstance = new MutexFactory();

MutexFactory::MutexFactory() {
}

MutexFactory::~MutexFactory() {
}

MutexFactory* MutexFactory::instance() {
	return MutexFactory::factoryInstance;
}

MutexIF* MutexFactory::createMutex() {
	return new Mutex();
}

void MutexFactory::deleteMutex(MutexIF* mutex) {
	delete mutex;
}

#include <framework/ipc/MutexFactory.h>
#include <framework/osal/FreeRTOS/Mutex.h>

//TODO: Different variant than the lazy loading in QueueFactory.
//What's better and why? -> one is on heap the other on bss/data
//MutexFactory* MutexFactory::factoryInstance = new MutexFactory();
MutexFactory* MutexFactory::factoryInstance = nullptr;

MutexFactory::MutexFactory() {
}

MutexFactory::~MutexFactory() {
}

MutexFactory* MutexFactory::instance() {
	if (factoryInstance == nullptr){
		factoryInstance = new MutexFactory();
	}
	return MutexFactory::factoryInstance;
}

MutexIF* MutexFactory::createMutex() {
	return new Mutex();
}

void MutexFactory::deleteMutex(MutexIF* mutex) {
	delete mutex;
}

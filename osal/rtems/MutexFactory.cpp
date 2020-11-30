#include "../../ipc/MutexFactory.h"
#include "Mutex.h"
#include "RtemsBasic.h"

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

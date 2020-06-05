#include <framework/datapoollocal/LocalDataSet.h>
#include <framework/housekeeping/HousekeepingManager.h>

LocalDataSet::LocalDataSet(HasHkPoolParametersIF *hkOwner): DataSetBase() {
	if(hkOwner != nullptr) {
		hkManager = hkOwner->getHkManagerHandle();
	}
	else {
		// config error, error output here.
	}
}

LocalDataSet::LocalDataSet(object_id_t ownerId): DataSetBase()  {
	HasHkPoolParametersIF* hkOwner = objectManager->get<HasHkPoolParametersIF>(
			ownerId);
	if(hkOwner == nullptr) {
		// config error, error output here.
	}
	hkManager = hkOwner->getHkManagerHandle();
}

LocalDataSet::~LocalDataSet() {
}

ReturnValue_t LocalDataSet::lockDataPool(uint32_t timeoutMs) {
	MutexIF* mutex = hkManager->getMutexHandle();
	return mutex->lockMutex(timeoutMs);
}

ReturnValue_t LocalDataSet::unlockDataPool() {
	MutexIF* mutex = hkManager->getMutexHandle();
	return mutex->unlockMutex();
}



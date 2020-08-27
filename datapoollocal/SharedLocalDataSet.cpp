#include "SharedLocalDataSet.h"

SharedLocalDataSet::SharedLocalDataSet(object_id_t objectId, sid_t sid,
        const size_t maxSize): SystemObject(objectId),
        LocalPoolDataSetBase(sid, nullptr, maxSize) {
    this->setContainer(poolVarVector.data());
    datasetLock = MutexFactory::instance()->createMutex();
}

ReturnValue_t SharedLocalDataSet::lockDataset(dur_millis_t mutexTimeout) {
    return datasetLock->lockMutex(MutexIF::TimeoutType::WAITING, mutexTimeout);
}

ReturnValue_t SharedLocalDataSet::unlockDataset() {
    return datasetLock->unlockMutex();
}

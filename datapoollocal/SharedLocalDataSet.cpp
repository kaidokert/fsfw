#include <framework/datapoollocal/SharedLocalDataSet.h>

SharedLocalDataSet::SharedLocalDataSet(object_id_t objectId, object_id_t owner,
        const size_t maxSize): SystemObject(objectId),
        LocalDataSetBase(owner, nullptr, maxSize) {
    this->setContainer(poolVarVector.data());
    datasetLock = MutexFactory::instance()->createMutex();
}

ReturnValue_t SharedLocalDataSet::lockDataset(dur_millis_t mutexTimeout) {
    return datasetLock->lockMutex(mutexTimeout);
}

ReturnValue_t SharedLocalDataSet::unlockDataset() {
    return datasetLock->unlockMutex();
}

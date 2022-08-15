#include "fsfw/datapoollocal/SharedLocalDataSet.h"

SharedLocalDataSet::SharedLocalDataSet(object_id_t objectId, sid_t sid, const size_t maxSize)
    : SystemObject(objectId), LocalPoolDataSetBase(sid, nullptr, maxSize), poolVarVector(maxSize) {
  this->setContainer(poolVarVector.data());
  datasetLock = MutexFactory::instance()->createMutex();
}

SharedLocalDataSet::SharedLocalDataSet(object_id_t objectId, HasLocalDataPoolIF *owner,
                                       uint32_t setId, const size_t maxSize)
    : SystemObject(objectId),
      LocalPoolDataSetBase(owner, setId, nullptr, maxSize),
      poolVarVector(maxSize) {
  this->setContainer(poolVarVector.data());
  datasetLock = MutexFactory::instance()->createMutex();
}

ReturnValue_t SharedLocalDataSet::lockDataset(MutexIF::TimeoutType timeoutType,
                                              dur_millis_t mutexTimeout) {
  if (datasetLock != nullptr) {
    return datasetLock->lockMutex(timeoutType, mutexTimeout);
  }
  return returnvalue::FAILED;
}

SharedLocalDataSet::~SharedLocalDataSet() { MutexFactory::instance()->deleteMutex(datasetLock); }

ReturnValue_t SharedLocalDataSet::unlockDataset() {
  if (datasetLock != nullptr) {
    return datasetLock->unlockMutex();
  }
  return returnvalue::FAILED;
}

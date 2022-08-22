#include "LocalPoolOwnerBase.h"

LocalPoolOwnerBase::LocalPoolOwnerBase(MessageQueueIF &queue, object_id_t objectId)
    : SystemObject(objectId),
      queue(queue),
      poolManager(this, &queue),
      dataset(this, lpool::testSetId) {}

LocalPoolOwnerBase::~LocalPoolOwnerBase() = default;

ReturnValue_t LocalPoolOwnerBase::initializeHkManager() {
  if (not initialized) {
    initialized = true;
    return poolManager.initialize(&queue);
  }
  return returnvalue::OK;
}

ReturnValue_t LocalPoolOwnerBase::initializeLocalDataPool(localpool::DataPool &localDataPoolMap,
                                                          LocalDataPoolManager &poolManager) {
  // Default initialization empty for now.
  localDataPoolMap.emplace(lpool::uint8VarId, &u8PoolEntry);
  localDataPoolMap.emplace(lpool::floatVarId, &floatPoolEntry);
  localDataPoolMap.emplace(lpool::uint32VarId, &u32PoolEntry);

  localDataPoolMap.emplace(lpool::uint16Vec3Id, &u16VecPoolEntry);
  localDataPoolMap.emplace(lpool::int64Vec2Id, &i64VecPoolEntry);
  return returnvalue::OK;
}

LocalPoolObjectBase *LocalPoolOwnerBase::getPoolObjectHandle(lp_id_t localPoolId) {
  if (localPoolId == lpool::uint8VarId) {
    return &testUint8;
  } else if (localPoolId == lpool::uint16Vec3Id) {
    return &testUint16Vec;
  } else if (localPoolId == lpool::floatVarId) {
    return &testFloat;
  } else if (localPoolId == lpool::int64Vec2Id) {
    return &testInt64Vec;
  } else if (localPoolId == lpool::uint32VarId) {
    return &testUint32;
  } else {
    return &testUint8;
  }
}

ReturnValue_t LocalPoolOwnerBase::reset() {
  resetSubscriptionList();
  ReturnValue_t status = returnvalue::OK;
  {
    PoolReadGuard readHelper(&dataset);
    if (readHelper.getReadResult() != returnvalue::OK) {
      status = readHelper.getReadResult();
    }
    dataset.localPoolVarUint8.value = 0;
    dataset.localPoolVarFloat.value = 0.0;
    dataset.localPoolUint16Vec.value[0] = 0;
    dataset.localPoolUint16Vec.value[1] = 0;
    dataset.localPoolUint16Vec.value[2] = 0;
    dataset.setValidity(false, true);
  }

  {
    PoolReadGuard readHelper(&testUint32);
    if (readHelper.getReadResult() != returnvalue::OK) {
      status = readHelper.getReadResult();
    }
    testUint32.value = 0;
    testUint32.setValid(false);
  }

  {
    PoolReadGuard readHelper(&testInt64Vec);
    if (readHelper.getReadResult() != returnvalue::OK) {
      status = readHelper.getReadResult();
    }
    testInt64Vec.value[0] = 0;
    testInt64Vec.value[1] = 0;
    testInt64Vec.setValid(false);
  }
  return status;
}

bool LocalPoolOwnerBase::changedDataSetCallbackWasCalled(sid_t &sid, store_address_t &storeId) {
  bool condition = false;
  if (not this->changedDatasetSid.notSet()) {
    condition = true;
  }
  sid = changedDatasetSid;
  storeId = storeIdForChangedSet;
  this->changedDatasetSid.raw = sid_t::INVALID_SID;
  this->storeIdForChangedSet = store_address_t::invalid();
  return condition;
}

void LocalPoolOwnerBase::handleChangedDataset(sid_t sid, store_address_t storeId,
                                              bool *clearMessage) {
  this->changedDatasetSid = sid;
  this->storeIdForChangedSet = storeId;
}

bool LocalPoolOwnerBase::changedVariableCallbackWasCalled(gp_id_t &gpid, store_address_t &storeId) {
  bool condition = false;
  if (not this->changedPoolVariableGpid.notSet()) {
    condition = true;
  }
  gpid = changedPoolVariableGpid;
  storeId = storeIdForChangedVariable;
  this->changedPoolVariableGpid.raw = gp_id_t::INVALID_GPID;
  this->storeIdForChangedVariable = store_address_t::invalid();
  return condition;
}

ReturnValue_t LocalPoolOwnerBase::initializeHkManagerAfterTaskCreation() {
  if (not initializedAfterTaskCreation) {
    initializedAfterTaskCreation = true;
    return poolManager.initializeAfterTaskCreation();
  }
  return returnvalue::OK;
}

void LocalPoolOwnerBase::handleChangedPoolVariable(gp_id_t globPoolId, store_address_t storeId,
                                                   bool *clearMessage) {
  this->changedPoolVariableGpid = globPoolId;
  this->storeIdForChangedVariable = storeId;
}

void LocalPoolOwnerBase::setHkDestId(MessageQueueId_t id) { poolManager.setHkDestinationId(id); }

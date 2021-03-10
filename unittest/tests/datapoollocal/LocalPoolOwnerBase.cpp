#include "LocalPoolOwnerBase.h"

LocalPoolOwnerBase::LocalPoolOwnerBase(object_id_t objectId):
        SystemObject(objectId), poolManager(this, messageQueue),
        dataset(this, lpool::testSetId) {
    messageQueue = new MessageQueueMockBase();
}

ReturnValue_t LocalPoolOwnerBase::initializeHkManager() {
    if(not initialized) {
        initialized = true;
        return poolManager.initialize(messageQueue);
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t LocalPoolOwnerBase::initializeLocalDataPool(localpool::DataPool &localDataPoolMap,
        LocalDataPoolManager &poolManager) {

    // Default initialization empty for now.
    localDataPoolMap.emplace(lpool::uint8VarId,
            new PoolEntry<uint8_t>({0}));
    localDataPoolMap.emplace(lpool::floatVarId,
            new PoolEntry<float>({0}));
    localDataPoolMap.emplace(lpool::uint32VarId,
            new PoolEntry<uint32_t>({0}));

    localDataPoolMap.emplace(lpool::uint16Vec3Id,
            new PoolEntry<uint16_t>({0, 0, 0}));
    localDataPoolMap.emplace(lpool::int64Vec2Id,
            new PoolEntry<int64_t>({0, 0}));
    return HasReturnvaluesIF::RETURN_OK;
}

LocalPoolObjectBase* LocalPoolOwnerBase::getPoolObjectHandle(lp_id_t localPoolId) {
    if(localPoolId == lpool::uint8VarId) {
        return &testUint8;
    }
    else if(localPoolId == lpool::uint16Vec3Id) {
        return &testUint16Vec;
    }
    else if(localPoolId == lpool::floatVarId) {
        return &testFloat;
    }
    else if(localPoolId == lpool::int64Vec2Id) {
        return &testInt64Vec;
    }
    else if(localPoolId == lpool::uint32VarId) {
        return &testUint32;
    }
    else {
        return &testUint8;
    }
}

ReturnValue_t LocalPoolOwnerBase::reset() {
    resetSubscriptionList();
    ReturnValue_t status = HasReturnvaluesIF::RETURN_OK;
    {
        PoolReadGuard readHelper(&dataset);
        if(readHelper.getReadResult() != HasReturnvaluesIF::RETURN_OK) {
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
        if(readHelper.getReadResult() != HasReturnvaluesIF::RETURN_OK) {
            status = readHelper.getReadResult();
        }
        testUint32.value = 0;
        testUint32.setValid(false);
    }

    {
        PoolReadGuard readHelper(&testInt64Vec);
        if(readHelper.getReadResult() != HasReturnvaluesIF::RETURN_OK) {
            status = readHelper.getReadResult();
        }
        testInt64Vec.value[0] = 0;
        testInt64Vec.value[1] = 0;
        testInt64Vec.setValid(false);
    }
    return status;
}

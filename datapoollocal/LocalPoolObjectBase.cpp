#include "LocalPoolObjectBase.h"

LocalPoolObjectBase::LocalPoolObjectBase(lp_id_t poolId,
        HasLocalDataPoolIF* hkOwner, DataSetIF* dataSet,
        pool_rwm_t setReadWriteMode): localPoolId(poolId),
        readWriteMode(setReadWriteMode) {
    if(poolId == PoolVariableIF::NO_PARAMETER) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "LocalPoolVar<T>::LocalPoolVar: 0 passed as pool ID, "
                << "which is the NO_PARAMETER value!" << std::endl;
#endif
    }
    if(hkOwner == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "LocalPoolVar<T>::LocalPoolVar: The supplied pool "
                << "owner is a invalid!" << std::endl;
#endif
        return;
    }
    hkManager = hkOwner->getHkManagerHandle();
    if (dataSet != nullptr) {
        dataSet->registerVariable(this);
    }
}

LocalPoolObjectBase::LocalPoolObjectBase(object_id_t poolOwner, lp_id_t poolId,
        DataSetIF *dataSet, pool_rwm_t setReadWriteMode): localPoolId(poolId),
        readWriteMode(setReadWriteMode) {
    if(poolId == PoolVariableIF::NO_PARAMETER) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::warning << "LocalPoolVar<T>::LocalPoolVar: 0 passed as pool ID, "
                << "which is the NO_PARAMETER value!" << std::endl;
#endif
    }
    HasLocalDataPoolIF* hkOwner =
            objectManager->get<HasLocalDataPoolIF>(poolOwner);
    if(hkOwner == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "LocalPoolVariable: The supplied pool owner did not "
                << "implement the correct interface"
                << " HasLocalDataPoolIF!" << std::endl;
#endif
        return;
    }
    hkManager = hkOwner->getHkManagerHandle();
    if(dataSet != nullptr) {
        dataSet->registerVariable(this);
    }
}

pool_rwm_t LocalPoolObjectBase::getReadWriteMode() const {
    return readWriteMode;
}

bool LocalPoolObjectBase::isValid() const {
    return valid;
}

void LocalPoolObjectBase::setValid(bool valid) {
    this->valid = valid;
}

lp_id_t LocalPoolObjectBase::getDataPoolId() const {
    return localPoolId;
}

void LocalPoolObjectBase::setDataPoolId(lp_id_t poolId) {
    this->localPoolId = poolId;
}

void LocalPoolObjectBase::setChanged(bool changed) {
    this->changed = changed;
}

bool LocalPoolObjectBase::hasChanged() const {
    return changed;
}

void LocalPoolObjectBase::setReadWriteMode(pool_rwm_t newReadWriteMode) {
	this->readWriteMode = newReadWriteMode;
}

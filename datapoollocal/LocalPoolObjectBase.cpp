#include "LocalPoolObjectBase.h"
#include "HasLocalDataPoolIFAttorney.h"

LocalPoolObjectBase::LocalPoolObjectBase(lp_id_t poolId, HasLocalDataPoolIF* hkOwner,
		DataSetIF* dataSet, pool_rwm_t setReadWriteMode):
		localPoolId(poolId), readWriteMode(setReadWriteMode) {
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
	AccessPoolManagerIF* poolManAccessor = HasLocalDpIFUserAttorney::getAccessorHandle(hkOwner);
	hkManager = poolManAccessor->getHkManagerHandle();
	//HasLohkOwner->getHkManagerHandle();
	if (dataSet != nullptr) {
		dataSet->registerVariable(this);
	}
}

LocalPoolObjectBase::LocalPoolObjectBase(object_id_t poolOwner, lp_id_t poolId, DataSetIF *dataSet,
		pool_rwm_t setReadWriteMode):
				localPoolId(poolId), readWriteMode(setReadWriteMode) {
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
	//hkManager = hkOwner->getHkManagerHandle();
	AccessPoolManagerIF* poolManAccessor = HasLocalDpIFUserAttorney::getAccessorHandle(hkOwner);
	//hkManager = HasLocalDpIFUserAttorney::getHkManagerHandle(hkOwner);
	hkManager = poolManAccessor->getHkManagerHandle();
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

void LocalPoolObjectBase::reportReadCommitError(const char* variableType,
		ReturnValue_t error, bool read, object_id_t objectId, lp_id_t lpId) {
#if FSFW_DISABLE_PRINTOUT == 0
	const char* type = nullptr;
	if(read) {
		type = "read";
	}
	else {
		type = "commit";
	}

	const char* errMsg = nullptr;
	if(error == HasLocalDataPoolIF::POOL_ENTRY_NOT_FOUND) {
		errMsg = "Pool entry not found";
	}
	else if(error == HasLocalDataPoolIF::POOL_ENTRY_TYPE_CONFLICT) {
		errMsg = "Pool entry type conflict";
	}
	else if(error == PoolVariableIF::INVALID_READ_WRITE_MODE) {
		errMsg = "Pool variable wrong read-write mode";
	}
	else if(error == PoolVariableIF::INVALID_POOL_ENTRY) {
		errMsg = "Pool entry invalid";
	}
	else {
		errMsg = "Unknown error code";
	}

#if FSFW_CPP_OSTREAM_ENABLED == 1
	sif::warning << variableType << ": " << type << " call | " << errMsg << " | Owner: 0x"
			<< std::hex << std::setw(8) << std::setfill('0') << objectId << std::dec
			<< " LPID: " << lpId << std::endl;
#else
	sif::printWarning("%s: %s call | %s | Owner: 0x%08x LPID: %lu\n",
			variableType, type, errMsg, objectId, lpId);
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* FSFW_DISABLE_PRINTOUT == 0 */
}

#ifndef FRAMEWORK_DATAPOOLLOCAL_LOCALPOOLVARIABLE_TPP_
#define FRAMEWORK_DATAPOOLLOCAL_LOCALPOOLVARIABLE_TPP_

#include <framework/housekeeping/HasHkPoolParametersIF.h>
#include <framework/objectmanager/ObjectManagerIF.h>

#include <framework/serialize/SerializeAdapter.h>

template<typename T>
inline LocalPoolVar<T>::LocalPoolVar(lp_id_t poolId,
		HasHkPoolParametersIF* hkOwner, pool_rwm_t setReadWriteMode,
		DataSetIF* dataSet):
		localPoolId(poolId),readWriteMode(setReadWriteMode) {
	hkManager = hkOwner->getHkManagerHandle();
	if(dataSet != nullptr) {
		dataSet->registerVariable(this);
	}
}

template<typename T>
inline LocalPoolVar<T>::LocalPoolVar(lp_id_t poolId, object_id_t poolOwner,
		pool_rwm_t setReadWriteMode, DataSetIF *dataSet):
		readWriteMode(readWriteMode) {
	HasHkPoolParametersIF* hkOwner =
			objectManager->get<HasHkPoolParametersIF>(poolOwner);
	if(hkOwner == nullptr) {
		sif::error << "LocalPoolVariable: The supplied pool owner did not implement"
				"the correct interface HasHkPoolParametersIF!" << std::endl;
		objectValid = false;
		return;
	}
	hkManager = hkOwner->getHkManagerHandle();
	if(dataSet != nullptr) {
		dataSet->registerVariable(this);
	}
}

template<typename T>
inline ReturnValue_t LocalPoolVar<T>::read() {
	if(readWriteMode == pool_rwm_t::VAR_WRITE) {
		sif::debug << "LocalPoolVar: Invalid read write "
				"mode for read() call." << std::endl;
		// TODO: special return value
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	MutexHelper(hkManager->getMutexHandle(), MutexIF::NO_TIMEOUT);
	PoolEntry<T>* poolEntry = nullptr;
	ReturnValue_t result = hkManager->fetchPoolEntry(localPoolId, poolEntry);
	if(result != RETURN_OK) {
		return result;
	}
	this->value = *(poolEntry->address);
	return RETURN_OK;
}

template<typename T>
inline ReturnValue_t LocalPoolVar<T>::commit() {
	if(readWriteMode == pool_rwm_t::VAR_READ) {
		sif::debug << "LocalPoolVar: Invalid read write "
				 "mode for commit() call." << std::endl;
		// TODO: special return value
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	MutexHelper(hkManager->getMutexHandle(), MutexIF::NO_TIMEOUT);
	PoolEntry<T>* poolEntry = nullptr;
	ReturnValue_t result = hkManager->fetchPoolEntry(localPoolId, poolEntry);
	if(result != RETURN_OK) {
		return result;
	}
	*(poolEntry->address) = this->value;
	return RETURN_OK;
}

template<typename T>
inline pool_rwm_t LocalPoolVar<T>::getReadWriteMode() const {
	return readWriteMode;
}

template<typename T>
inline lp_id_t LocalPoolVar<T>::getDataPoolId() const {
	return localPoolId;
}

template<typename T>
inline bool LocalPoolVar<T>::isValid() const {
	return valid;
}

template<typename T>
inline void LocalPoolVar<T>::setValid(uint8_t validity) {
	this->valid = validity;
}

template<typename T>
inline ReturnValue_t LocalPoolVar<T>::serialize(uint8_t** buffer, size_t* size,
		const size_t max_size, bool bigEndian) const {
	return AutoSerializeAdapter::serialize(&value,
			buffer, size ,max_size, bigEndian);
}

template<typename T>
inline size_t LocalPoolVar<T>::getSerializedSize() const {
	return AutoSerializeAdapter::getSerializedSize(&value);
}

template<typename T>
inline ReturnValue_t LocalPoolVar<T>::deSerialize(const uint8_t** buffer,
		size_t* size, bool bigEndian) {
	return AutoSerializeAdapter::deSerialize(&value, buffer, size, bigEndian);
}

#endif

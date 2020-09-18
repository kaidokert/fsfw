#ifndef FRAMEWORK_DATAPOOLLOCAL_LOCALPOOLVARIABLE_TPP_
#define FRAMEWORK_DATAPOOLLOCAL_LOCALPOOLVARIABLE_TPP_

#ifndef FRAMEWORK_DATAPOOLLOCAL_LOCALPOOLVARIABLE_H_
#error Include LocalPoolVariable.h before LocalPoolVariable.tpp!
#endif

template<typename T>
inline LocalPoolVar<T>::LocalPoolVar(lp_id_t poolId,
		HasLocalDataPoolIF* hkOwner, DataSetIF* dataSet,
		pool_rwm_t setReadWriteMode):
		localPoolId(poolId),readWriteMode(setReadWriteMode) {
	if(poolId == PoolVariableIF::NO_PARAMETER) {
		sif::warning << "LocalPoolVector: 0 passed as pool ID, which is the "
				"NO_PARAMETER value!" << std::endl;
	}
	if(hkOwner == nullptr) {
		sif::error << "LocalPoolVariable: The supplied pool owner is a nullptr!"
				<< std::endl;
		return;
	}
	hkManager = hkOwner->getHkManagerHandle();
	if(dataSet != nullptr) {
		dataSet->registerVariable(this);
	}
}

template<typename T>
inline LocalPoolVar<T>::LocalPoolVar(lp_id_t poolId, object_id_t poolOwner,
		DataSetIF *dataSet, pool_rwm_t setReadWriteMode):
		readWriteMode(setReadWriteMode) {
	if(poolId == PoolVariableIF::NO_PARAMETER) {
		sif::warning << "LocalPoolVector: 0 passed as pool ID, which is the "
				"NO_PARAMETER value!" << std::endl;
	}
	HasLocalDataPoolIF* hkOwner =
			objectManager->get<HasLocalDataPoolIF>(poolOwner);
	if(hkOwner == nullptr) {
		sif::error << "LocalPoolVariable: The supplied pool owner did not implement"
				"the correct interface HasHkPoolParametersIF!" << std::endl;
		return;
	}
	hkManager = hkOwner->getHkManagerHandle();
	if(dataSet != nullptr) {
		dataSet->registerVariable(this);
	}
}

template<typename T>
inline ReturnValue_t LocalPoolVar<T>::read(dur_millis_t lockTimeout) {
	MutexHelper(hkManager->getMutexHandle(), MutexIF::TimeoutType::WAITING,
			lockTimeout);
	return readWithoutLock();
}

template<typename T>
inline ReturnValue_t LocalPoolVar<T>::readWithoutLock() {
	if(readWriteMode == pool_rwm_t::VAR_WRITE) {
		sif::debug << "LocalPoolVar: Invalid read write "
				"mode for read() call." << std::endl;
		return PoolVariableIF::INVALID_READ_WRITE_MODE;
	}

	PoolEntry<T>* poolEntry = nullptr;
	ReturnValue_t result = hkManager->fetchPoolEntry(localPoolId, &poolEntry);
	if(result != RETURN_OK and poolEntry != nullptr) {
		sif::error << "PoolVector: Read of local pool variable of object "
				"0x" << std::hex << std::setw(8) << std::setfill('0') <<
				hkManager->getOwner() << " and lp ID 0x" << localPoolId <<
				std::dec << " failed.\n" << std::flush;
		return result;
	}
	this->value = *(poolEntry->address);
	this->valid = poolEntry->valid;
	return RETURN_OK;
}

template<typename T>
inline ReturnValue_t LocalPoolVar<T>::commit(dur_millis_t lockTimeout) {
	MutexHelper(hkManager->getMutexHandle(), MutexIF::TimeoutType::WAITING,
			lockTimeout);
	return commitWithoutLock();
}

template<typename T>
inline ReturnValue_t LocalPoolVar<T>::commitWithoutLock() {
	if(readWriteMode == pool_rwm_t::VAR_READ) {
		sif::debug << "LocalPoolVar: Invalid read write "
				 "mode for commit() call." << std::endl;
		return PoolVariableIF::INVALID_READ_WRITE_MODE;
	}
	PoolEntry<T>* poolEntry = nullptr;
	ReturnValue_t result = hkManager->fetchPoolEntry(localPoolId, &poolEntry);
	if(result != RETURN_OK) {
		sif::error << "PoolVector: Read of local pool variable of object "
				"0x" << std::hex << std::setw(8) << std::setfill('0') <<
				hkManager->getOwner() << " and lp ID 0x" << localPoolId <<
				std::dec << " failed.\n" << std::flush;
		return result;
	}
	*(poolEntry->address) = this->value;
	poolEntry->valid = this->valid;
	return RETURN_OK;
}

template<typename T>
inline LocalPoolVar<T> & LocalPoolVar<T>::operator =(T newValue) {
    value = newValue;
    return *this;
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
inline void LocalPoolVar<T>::setDataPoolId(lp_id_t poolId) {
	this->localPoolId = poolId;
}

template<typename T>
inline bool LocalPoolVar<T>::isValid() const {
	return valid;
}

template<typename T>
inline void LocalPoolVar<T>::setValid(bool validity) {
	this->valid = validity;
}

template<typename T>
inline uint8_t LocalPoolVar<T>::getValid() const {
	return valid;
}

template<typename T>
inline ReturnValue_t LocalPoolVar<T>::serialize(uint8_t** buffer, size_t* size,
		const size_t max_size, SerializeIF::Endianness streamEndianness) const {
	return SerializeAdapter::serialize(&value,
			buffer, size ,max_size, streamEndianness);
}

template<typename T>
inline size_t LocalPoolVar<T>::getSerializedSize() const {
	return SerializeAdapter::getSerializedSize(&value);
}

template<typename T>
inline ReturnValue_t LocalPoolVar<T>::deSerialize(const uint8_t** buffer,
		size_t* size, SerializeIF::Endianness streamEndianness) {
	return SerializeAdapter::deSerialize(&value, buffer, size, streamEndianness);
}

template<typename T>
inline std::ostream& operator<< (std::ostream &out,
		const LocalPoolVar<T> &var) {
    out << var.value;
    return out;
}

#endif

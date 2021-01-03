#ifndef FSFW_DATAPOOLLOCAL_LOCALPOOLVARIABLE_TPP_
#define FSFW_DATAPOOLLOCAL_LOCALPOOLVARIABLE_TPP_

#ifndef FSFW_DATAPOOLLOCAL_LOCALPOOLVARIABLE_H_
#error Include LocalPoolVariable.h before LocalPoolVariable.tpp!
#endif

template<typename T>
inline LocalPoolVariable<T>::LocalPoolVariable(HasLocalDataPoolIF* hkOwner,
		lp_id_t poolId, DataSetIF* dataSet, pool_rwm_t setReadWriteMode):
		LocalPoolObjectBase(poolId, hkOwner, dataSet, setReadWriteMode) {}

template<typename T>
inline LocalPoolVariable<T>::LocalPoolVariable(object_id_t poolOwner,
		lp_id_t poolId, DataSetIF *dataSet, pool_rwm_t setReadWriteMode):
        LocalPoolObjectBase(poolOwner, poolId, dataSet, setReadWriteMode) {}


template<typename T>
inline LocalPoolVariable<T>::LocalPoolVariable(gp_id_t globalPoolId,
		DataSetIF *dataSet, pool_rwm_t setReadWriteMode):
		LocalPoolObjectBase(globalPoolId.objectId, globalPoolId.localPoolId,
				dataSet, setReadWriteMode){}


template<typename T>
inline ReturnValue_t LocalPoolVariable<T>::read(dur_millis_t lockTimeout) {
	MutexHelper(hkManager->getMutexHandle(), MutexIF::TimeoutType::WAITING,
			lockTimeout);
	return readWithoutLock();
}

template<typename T>
inline ReturnValue_t LocalPoolVariable<T>::readWithoutLock() {
	if(readWriteMode == pool_rwm_t::VAR_WRITE) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::debug << "LocalPoolVar: Invalid read write "
				"mode for read() call." << std::endl;
#endif
		return PoolVariableIF::INVALID_READ_WRITE_MODE;
	}

	PoolEntry<T>* poolEntry = nullptr;
	ReturnValue_t result = hkManager->fetchPoolEntry(localPoolId, &poolEntry);
	if(result != RETURN_OK or poolEntry == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "PoolVector: Read of local pool variable of object "
				<< std::hex << std::setw(8) << std::setfill('0')
				<< hkManager->getOwner() << " and lp ID " << localPoolId
				<< std::dec << " failed." << std::setfill(' ') <<  std::endl;
#endif
		return result;
	}
	this->value = *(poolEntry->address);
	this->valid = poolEntry->valid;
	return RETURN_OK;
}

template<typename T>
inline ReturnValue_t LocalPoolVariable<T>::commit(dur_millis_t lockTimeout) {
	MutexHelper(hkManager->getMutexHandle(), MutexIF::TimeoutType::WAITING,
			lockTimeout);
	return commitWithoutLock();
}

template<typename T>
inline ReturnValue_t LocalPoolVariable<T>::commitWithoutLock() {
	if(readWriteMode == pool_rwm_t::VAR_READ) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::debug << "LocalPoolVariable: Invalid read write "
				 "mode for commit() call." << std::endl;
#endif
		return PoolVariableIF::INVALID_READ_WRITE_MODE;
	}
	PoolEntry<T>* poolEntry = nullptr;
	ReturnValue_t result = hkManager->fetchPoolEntry(localPoolId, &poolEntry);
	if(result != RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "PoolVector: Read of local pool variable of object "
				"0x" << std::hex << std::setw(8) << std::setfill('0') <<
				hkManager->getOwner() << " and lp ID 0x" << localPoolId <<
				std::dec << " failed.\n" << std::flush;
#endif
		return result;
	}
	*(poolEntry->address) = this->value;
	poolEntry->valid = this->valid;
	return RETURN_OK;
}

template<typename T>
inline ReturnValue_t LocalPoolVariable<T>::serialize(uint8_t** buffer, size_t* size,
		const size_t max_size, SerializeIF::Endianness streamEndianness) const {
	return SerializeAdapter::serialize(&value,
			buffer, size ,max_size, streamEndianness);
}

template<typename T>
inline size_t LocalPoolVariable<T>::getSerializedSize() const {
	return SerializeAdapter::getSerializedSize(&value);
}

template<typename T>
inline ReturnValue_t LocalPoolVariable<T>::deSerialize(const uint8_t** buffer,
		size_t* size, SerializeIF::Endianness streamEndianness) {
	return SerializeAdapter::deSerialize(&value, buffer, size, streamEndianness);
}

#if FSFW_CPP_OSTREAM_ENABLED == 1
template<typename T>
inline std::ostream& operator<< (std::ostream &out,
		const LocalPoolVariable<T> &var) {
    out << var.value;
    return out;
}
#endif

template<typename T>
inline LocalPoolVariable<T>::operator T() const {
	return value;
}

template<typename T>
inline LocalPoolVariable<T> & LocalPoolVariable<T>::operator=(const T& newValue) {
    value = newValue;
    return *this;
}

template<typename T>
inline LocalPoolVariable<T>& LocalPoolVariable<T>::operator =(
		const LocalPoolVariable<T>& newPoolVariable) {
	value = newPoolVariable.value;
	return *this;
}

template<typename T>
inline bool LocalPoolVariable<T>::operator ==(const LocalPoolVariable<T> &other) const {
	return this->value == other.value;
}

template<typename T>
inline bool LocalPoolVariable<T>::operator ==(const T &other) const {
	return this->value == other;
}


template<typename T>
inline bool LocalPoolVariable<T>::operator !=(const LocalPoolVariable<T> &other) const {
	return not (*this == other);
}

template<typename T>
inline bool LocalPoolVariable<T>::operator !=(const T &other) const {
	return not (*this == other);
}


template<typename T>
inline bool LocalPoolVariable<T>::operator <(const LocalPoolVariable<T> &other) const {
	return this->value < other.value;
}

template<typename T>
inline bool LocalPoolVariable<T>::operator <(const T &other) const {
	return this->value < other;
}


template<typename T>
inline bool LocalPoolVariable<T>::operator >(const LocalPoolVariable<T> &other) const {
	return not (*this < other);
}

template<typename T>
inline bool LocalPoolVariable<T>::operator >(const T &other) const {
	return not (*this < other);
}

#endif /* FSFW_DATAPOOLLOCAL_LOCALPOOLVARIABLE_TPP_ */

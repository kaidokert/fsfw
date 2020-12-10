#ifndef FSFW_DATAPOOLLOCAL_LOCALPOOLVECTOR_TPP_
#define FSFW_DATAPOOLLOCAL_LOCALPOOLVECTOR_TPP_

#ifndef FSFW_DATAPOOLLOCAL_LOCALPOOLVECTOR_H_
#error Include LocalPoolVector.h before LocalPoolVector.tpp!
#endif

template<typename T, uint16_t vectorSize>
inline LocalPoolVector<T, vectorSize>::LocalPoolVector(
		HasLocalDataPoolIF* hkOwner, lp_id_t poolId, DataSetIF* dataSet,
		pool_rwm_t setReadWriteMode):
		LocalPoolObjectBase(poolId, hkOwner, dataSet, setReadWriteMode) {}

template<typename T, uint16_t vectorSize>
inline LocalPoolVector<T, vectorSize>::LocalPoolVector(object_id_t poolOwner,
		lp_id_t poolId, DataSetIF *dataSet, pool_rwm_t setReadWriteMode):
		LocalPoolObjectBase(poolOwner, poolId, dataSet, setReadWriteMode) {}


template<typename T, uint16_t vectorSize>
inline LocalPoolVector<T, vectorSize>::LocalPoolVector(gp_id_t globalPoolId,
		DataSetIF *dataSet, pool_rwm_t setReadWriteMode):
		LocalPoolObjectBase(globalPoolId.objectId, globalPoolId.localPoolId,
				dataSet, setReadWriteMode) {}

template<typename T, uint16_t vectorSize>
inline ReturnValue_t LocalPoolVector<T, vectorSize>::read(uint32_t lockTimeout) {
	MutexHelper(hkManager->getMutexHandle(), MutexIF::TimeoutType::WAITING,
			lockTimeout);
	return readWithoutLock();
}
template<typename T, uint16_t vectorSize>
inline ReturnValue_t LocalPoolVector<T, vectorSize>::readWithoutLock() {
	if(readWriteMode == pool_rwm_t::VAR_WRITE) {
		sif::debug << "LocalPoolVar: Invalid read write "
				"mode for read() call." << std::endl;
		return PoolVariableIF::INVALID_READ_WRITE_MODE;
	}

	PoolEntry<T>* poolEntry = nullptr;
	ReturnValue_t result = hkManager->fetchPoolEntry(localPoolId, &poolEntry);
	memset(this->value, 0, vectorSize * sizeof(T));

	if(result != RETURN_OK) {
		sif::error << "PoolVector: Read of local pool variable of object "
				"0x" << std::hex << std::setw(8) << std::setfill('0') <<
				hkManager->getOwner() << "and lp ID 0x" << localPoolId <<
				std::dec << " failed." << std::endl;
		return result;
	}
	std::memcpy(this->value, poolEntry->address, poolEntry->getByteSize());
	this->valid = poolEntry->valid;
	return RETURN_OK;
}

template<typename T, uint16_t vectorSize>
inline ReturnValue_t LocalPoolVector<T, vectorSize>::commit(
		uint32_t lockTimeout) {
	MutexHelper(hkManager->getMutexHandle(), MutexIF::TimeoutType::WAITING,
			lockTimeout);
	return commitWithoutLock();
}

template<typename T, uint16_t vectorSize>
inline ReturnValue_t LocalPoolVector<T, vectorSize>::commitWithoutLock() {
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
	std::memcpy(poolEntry->address, this->value, poolEntry->getByteSize());
	poolEntry->valid = this->valid;
	return RETURN_OK;
}

template<typename T, uint16_t vectorSize>
inline T& LocalPoolVector<T, vectorSize>::operator [](int i) {
	if(i <= vectorSize) {
		return value[i];
	}
	// If this happens, I have to set some value. I consider this
	// a configuration error, but I wont exit here.
	sif::error << "LocalPoolVector: Invalid index. Setting or returning"
			" last value!" << std::endl;
	return value[i];
}

template<typename T, uint16_t vectorSize>
inline const T& LocalPoolVector<T, vectorSize>::operator [](int i) const {
	if(i <= vectorSize) {
		return value[i];
	}
	// If this happens, I have to set some value. I consider this
	// a configuration error, but I wont exit here.
	sif::error << "LocalPoolVector: Invalid index. Setting or returning"
			" last value!" << std::endl;
	return value[i];
}

template<typename T, uint16_t vectorSize>
inline ReturnValue_t LocalPoolVector<T, vectorSize>::serialize(uint8_t** buffer,
		size_t* size, size_t maxSize,
		SerializeIF::Endianness streamEndianness) const {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
	for (uint16_t i = 0; i < vectorSize; i++) {
		result = SerializeAdapter::serialize(&(value[i]), buffer, size,
				maxSize, streamEndianness);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			break;
		}
	}
	return result;
}

template<typename T, uint16_t vectorSize>
inline size_t LocalPoolVector<T, vectorSize>::getSerializedSize() const {
	return vectorSize * SerializeAdapter::getSerializedSize(value);
}

template<typename T, uint16_t vectorSize>
inline ReturnValue_t LocalPoolVector<T, vectorSize>::deSerialize(
		const uint8_t** buffer, size_t* size,
		SerializeIF::Endianness streamEndianness) {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
	for (uint16_t i = 0; i < vectorSize; i++) {
		result = SerializeAdapter::deSerialize(&(value[i]), buffer, size,
				streamEndianness);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			break;
		}
	}
	return result;
}

template<typename T, uint16_t vectorSize>
inline std::ostream& operator<< (std::ostream &out,
        const LocalPoolVector<T, vectorSize> &var) {
    out << "Vector: [";
    for(int i = 0;i < vectorSize; i++) {
        out << var.value[i];
        if(i < vectorSize - 1) {
            out << ", ";
        }
    }
    out << "]";
    return out;
}

#endif /* FSFW_DATAPOOLLOCAL_LOCALPOOLVECTOR_TPP_ */

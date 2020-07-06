#ifndef GLOBALPOOLVECTOR_TPP_
#define GLOBALPOOLVECTOR_TPP_


template<typename T, uint16_t vectorSize>
inline GlobPoolVector<T, vectorSize>::GlobPoolVector(uint32_t set_id,
		DataSetIF* set, ReadWriteMode_t setReadWriteMode) :
		dataPoolId(set_id), valid(false), readWriteMode(setReadWriteMode) {
	memset(this->value, 0, vectorSize * sizeof(T));
	if (set != nullptr) {
		set->registerVariable(this);
	}
}


template<typename T, uint16_t vectorSize>
inline ReturnValue_t GlobPoolVector<T, vectorSize>::read(uint32_t lockTimeout) {
	ReturnValue_t result = glob::dataPool.lockDataPool(lockTimeout);
	if(result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	result = readWithoutLock();
	ReturnValue_t unlockResult = glob::dataPool.unlockDataPool();
	if(unlockResult != HasReturnvaluesIF::RETURN_OK) {
		sif::error << "GlobPoolVar::read: Could not unlock global data pool"
				<< std::endl;
	}
	return result;
}

template<typename T, uint16_t vectorSize>
inline ReturnValue_t GlobPoolVector<T, vectorSize>::commit(
		uint32_t lockTimeout) {
	ReturnValue_t result = glob::dataPool.lockDataPool(lockTimeout);
	if(result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	result = commitWithoutLock();
	ReturnValue_t unlockResult = glob::dataPool.unlockDataPool();
	if(unlockResult != HasReturnvaluesIF::RETURN_OK) {
		sif::error << "GlobPoolVar::read: Could not unlock global data pool"
				<< std::endl;
	}
	return result;
}

template<typename T, uint16_t vectorSize>
inline ReturnValue_t GlobPoolVector<T, vectorSize>::readWithoutLock() {
	PoolEntry<T>* read_out = glob::dataPool.getData<T>(this->dataPoolId,
			vectorSize);
	if (read_out != nullptr) {
		this->valid = read_out->valid;
		memcpy(this->value, read_out->address, read_out->getByteSize());

		return HasReturnvaluesIF::RETURN_OK;

	} else {
		memset(this->value, 0, vectorSize * sizeof(T));
		sif::error << "PoolVector: Read of DP Variable 0x" << std::hex
				<< std::setw(8) << std::setfill('0') << dataPoolId <<
				std::dec << " failed." << std::endl;
		this->valid = INVALID;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

template<typename T, uint16_t vectorSize>
inline ReturnValue_t GlobPoolVector<T, vectorSize>::commitWithoutLock() {
	PoolEntry<T>* writeBack = glob::dataPool.getData<T>(this->dataPoolId,
			vectorSize);
	if ((writeBack != nullptr) && (this->readWriteMode != VAR_READ)) {
		writeBack->valid = valid;
		memcpy(writeBack->address, this->value, writeBack->getByteSize());
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

template<typename T, uint16_t vectorSize>
inline ReturnValue_t GlobPoolVector<T, vectorSize>::serialize(uint8_t** buffer,
		size_t* size, size_t max_size,
		SerializeIF::Endianness streamEndianness) const {
	uint16_t i;
	ReturnValue_t result;
	for (i = 0; i < vectorSize; i++) {
		result = SerializeAdapter::serialize(&(value[i]), buffer, size,
				max_size, streamEndianness);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
	}
	return result;
}

template<typename T, uint16_t vectorSize>
inline size_t GlobPoolVector<T, vectorSize>::getSerializedSize() const {
	return vectorSize * SerializeAdapter::getSerializedSize(value);
}

template<typename T, uint16_t vectorSize>
inline ReturnValue_t GlobPoolVector<T, vectorSize>::deSerialize(
		const uint8_t** buffer, size_t* size,
		SerializeIF::Endianness streamEndianness) {
	uint16_t i;
	ReturnValue_t result;
	for (i = 0; i < vectorSize; i++) {
		result = SerializeAdapter::deSerialize(&(value[i]), buffer, size,
		        streamEndianness);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return result;
		}
	}
	return result;
}

#endif

#pragma once

template <class T>
inline GlobPoolVar<T>::GlobPoolVar(uint32_t set_id,
		DataSetIF* dataSet, ReadWriteMode_t setReadWriteMode):
		dataPoolId(set_id), valid(PoolVariableIF::INVALID),
		readWriteMode(setReadWriteMode)
{
	if (dataSet != nullptr) {
		dataSet->registerVariable(this);
	}
}

template <class T>
inline ReturnValue_t GlobPoolVar<T>::read() {
	PoolEntry<T>* read_out = ::dataPool.getData<T>(dataPoolId, 1);
	if (read_out != NULL) {
		valid = read_out->valid;
		value = *(read_out->address);
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		value = 0;
		valid = false;
		sif::error << "PoolVariable: read of DP Variable 0x" << std::hex
				<< dataPoolId << std::dec << " failed." << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

template <class T>
inline ReturnValue_t GlobPoolVar<T>::commit() {
	PoolEntry<T>* write_back = ::dataPool.getData<T>(dataPoolId, 1);
	if ((write_back != NULL) && (readWriteMode != VAR_READ)) {
		write_back->valid = valid;
		*(write_back->address) = value;
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

template <class T>
inline GlobPoolVar<T>::GlobPoolVar():
		dataPoolId(PoolVariableIF::NO_PARAMETER),
		valid(PoolVariableIF::INVALID),
		readWriteMode(VAR_READ), value(0) {}

template <class T>
inline GlobPoolVar<T>::GlobPoolVar(const GlobPoolVar& rhs) :
		dataPoolId(rhs.dataPoolId), valid(rhs.valid), readWriteMode(
		rhs.readWriteMode), value(rhs.value) {}

template <class T>
inline pool_rwm_t GlobPoolVar<T>::getReadWriteMode() const  {
	return readWriteMode;
}

template <class T>
inline uint32_t GlobPoolVar<T>::getDataPoolId() const {
	return dataPoolId;
}

template <class T>
inline void GlobPoolVar<T>::setDataPoolId(uint32_t poolId) {
	dataPoolId = poolId;
}

template <class T>
inline bool GlobPoolVar<T>::isValid() const {
	if (valid)
		return true;
	else
		return false;
}

template <class T>
inline uint8_t GlobPoolVar<T>::getValid() {
	return valid;
}

template <class T>
inline void GlobPoolVar<T>::setValid(uint8_t valid) {
	this->valid = valid;
}

#ifndef FSFW_DATAPOOLLOCAL_LOCALPOOLVARIABLE_TPP_
#define FSFW_DATAPOOLLOCAL_LOCALPOOLVARIABLE_TPP_

#ifndef FSFW_DATAPOOLLOCAL_LOCALPOOLVARIABLE_H_
#error Include LocalPoolVariable.h before LocalPoolVariable.tpp!
#endif

template <typename T>
inline LocalPoolVariable<T>::LocalPoolVariable(HasLocalDataPoolIF* hkOwner, lp_id_t poolId,
                                               DataSetIF* dataSet, pool_rwm_t setReadWriteMode)
    : LocalPoolObjectBase(poolId, hkOwner, dataSet, setReadWriteMode) {}

template <typename T>
inline LocalPoolVariable<T>::LocalPoolVariable(object_id_t poolOwner, lp_id_t poolId,
                                               DataSetIF* dataSet, pool_rwm_t setReadWriteMode)
    : LocalPoolObjectBase(poolOwner, poolId, dataSet, setReadWriteMode) {}

template <typename T>
inline LocalPoolVariable<T>::LocalPoolVariable(gp_id_t globalPoolId, DataSetIF* dataSet,
                                               pool_rwm_t setReadWriteMode)
    : LocalPoolObjectBase(globalPoolId.objectId, globalPoolId.localPoolId, dataSet,
                          setReadWriteMode) {}

template <typename T>
inline ReturnValue_t LocalPoolVariable<T>::read(MutexIF::TimeoutType timeoutType,
                                                uint32_t timeoutMs) {
  if (hkManager == nullptr) {
    return readWithoutLock();
  }
  MutexIF* mutex = LocalDpManagerAttorney::getMutexHandle(*hkManager);
  ReturnValue_t result = mutex->lockMutex(timeoutType, timeoutMs);
  if (result != returnvalue::OK) {
    return result;
  }
  result = readWithoutLock();
  mutex->unlockMutex();
  return result;
}

template <typename T>
inline ReturnValue_t LocalPoolVariable<T>::readWithoutLock() {
  if (readWriteMode == pool_rwm_t::VAR_WRITE) {
    object_id_t targetObjectId = hkManager->getCreatorObjectId();
    reportReadCommitError("LocalPoolVector", PoolVariableIF::INVALID_READ_WRITE_MODE, true,
                          targetObjectId, localPoolId);
    return PoolVariableIF::INVALID_READ_WRITE_MODE;
  }

  PoolEntry<T>* poolEntry = nullptr;
  ReturnValue_t result =
      LocalDpManagerAttorney::fetchPoolEntry(*hkManager, localPoolId, &poolEntry);
  if (result != returnvalue::OK) {
    object_id_t ownerObjectId = hkManager->getCreatorObjectId();
    reportReadCommitError("LocalPoolVariable", result, false, ownerObjectId, localPoolId);
    return result;
  }

  this->value = *(poolEntry->getDataPtr());
  this->valid = poolEntry->getValid();
  return returnvalue::OK;
}

template <typename T>
inline ReturnValue_t LocalPoolVariable<T>::commit(bool setValid, MutexIF::TimeoutType timeoutType,
                                                  uint32_t timeoutMs) {
  this->setValid(setValid);
  return commit(timeoutType, timeoutMs);
}

template <typename T>
inline ReturnValue_t LocalPoolVariable<T>::commit(MutexIF::TimeoutType timeoutType,
                                                  uint32_t timeoutMs) {
  if (hkManager == nullptr) {
    return commitWithoutLock();
  }
  MutexIF* mutex = LocalDpManagerAttorney::getMutexHandle(*hkManager);
  ReturnValue_t result = mutex->lockMutex(timeoutType, timeoutMs);
  if (result != returnvalue::OK) {
    return result;
  }
  result = commitWithoutLock();
  mutex->unlockMutex();
  return result;
}

template <typename T>
inline ReturnValue_t LocalPoolVariable<T>::commitWithoutLock() {
  if (readWriteMode == pool_rwm_t::VAR_READ) {
    object_id_t targetObjectId = hkManager->getCreatorObjectId();
    reportReadCommitError("LocalPoolVector", PoolVariableIF::INVALID_READ_WRITE_MODE, false,
                          targetObjectId, localPoolId);
    return PoolVariableIF::INVALID_READ_WRITE_MODE;
  }

  PoolEntry<T>* poolEntry = nullptr;
  ReturnValue_t result =
      LocalDpManagerAttorney::fetchPoolEntry(*hkManager, localPoolId, &poolEntry);
  if (result != returnvalue::OK) {
    object_id_t ownerObjectId = hkManager->getCreatorObjectId();
    reportReadCommitError("LocalPoolVariable", result, false, ownerObjectId, localPoolId);
    return result;
  }

  *(poolEntry->getDataPtr()) = this->value;
  poolEntry->setValid(this->valid);
  return returnvalue::OK;
}

template <typename T>
inline ReturnValue_t LocalPoolVariable<T>::serialize(
    uint8_t** buffer, size_t* size, const size_t max_size,
    SerializeIF::Endianness streamEndianness) const {
  return SerializeAdapter::serialize(&value, buffer, size, max_size, streamEndianness);
}

template <typename T>
inline size_t LocalPoolVariable<T>::getSerializedSize() const {
  return SerializeAdapter::getSerializedSize(&value);
}

template <typename T>
inline ReturnValue_t LocalPoolVariable<T>::deSerialize(const uint8_t** buffer, size_t* size,
                                                       SerializeIF::Endianness streamEndianness) {
  return SerializeAdapter::deSerialize(&value, buffer, size, streamEndianness);
}

#if FSFW_CPP_OSTREAM_ENABLED == 1
template <typename T>
inline std::ostream& operator<<(std::ostream& out, const LocalPoolVariable<T>& var) {
  out << var.value;
  return out;
}
#endif

template <typename T>
inline LocalPoolVariable<T>::operator T() const {
  return value;
}

template <typename T>
inline LocalPoolVariable<T>& LocalPoolVariable<T>::operator=(const T& newValue) {
  value = newValue;
  return *this;
}

template <typename T>
inline LocalPoolVariable<T>& LocalPoolVariable<T>::operator=(
    const LocalPoolVariable<T>& newPoolVariable) {
  value = newPoolVariable.value;
  return *this;
}

template <typename T>
inline bool LocalPoolVariable<T>::operator==(const LocalPoolVariable<T>& other) const {
  return this->value == other.value;
}

template <typename T>
inline bool LocalPoolVariable<T>::operator==(const T& other) const {
  return this->value == other;
}

template <typename T>
inline bool LocalPoolVariable<T>::operator!=(const LocalPoolVariable<T>& other) const {
  return not(*this == other);
}

template <typename T>
inline bool LocalPoolVariable<T>::operator!=(const T& other) const {
  return not(*this == other);
}

template <typename T>
inline bool LocalPoolVariable<T>::operator<(const LocalPoolVariable<T>& other) const {
  return this->value < other.value;
}

template <typename T>
inline bool LocalPoolVariable<T>::operator<(const T& other) const {
  return this->value < other;
}

template <typename T>
inline bool LocalPoolVariable<T>::operator>(const LocalPoolVariable<T>& other) const {
  return not(*this < other);
}

template <typename T>
inline bool LocalPoolVariable<T>::operator>(const T& other) const {
  return not(*this < other);
}

#endif /* FSFW_DATAPOOLLOCAL_LOCALPOOLVARIABLE_TPP_ */

#ifndef FSFW_DATAPOOLLOCAL_LOCALPOOLVECTOR_TPP_
#define FSFW_DATAPOOLLOCAL_LOCALPOOLVECTOR_TPP_

#ifndef FSFW_DATAPOOLLOCAL_LOCALPOOLVECTOR_H_
#error Include LocalPoolVector.h before LocalPoolVector.tpp!
#endif

template <typename T, uint16_t vectorSize>
inline LocalPoolVector<T, vectorSize>::LocalPoolVector(HasLocalDataPoolIF* hkOwner, lp_id_t poolId,
                                                       DataSetIF* dataSet,
                                                       pool_rwm_t setReadWriteMode)
    : LocalPoolObjectBase(poolId, hkOwner, dataSet, setReadWriteMode) {}

template <typename T, uint16_t vectorSize>
inline LocalPoolVector<T, vectorSize>::LocalPoolVector(object_id_t poolOwner, lp_id_t poolId,
                                                       DataSetIF* dataSet,
                                                       pool_rwm_t setReadWriteMode)
    : LocalPoolObjectBase(poolOwner, poolId, dataSet, setReadWriteMode) {}

template <typename T, uint16_t vectorSize>
inline LocalPoolVector<T, vectorSize>::LocalPoolVector(gp_id_t globalPoolId, DataSetIF* dataSet,
                                                       pool_rwm_t setReadWriteMode)
    : LocalPoolObjectBase(globalPoolId.objectId, globalPoolId.localPoolId, dataSet,
                          setReadWriteMode) {}

template <typename T, uint16_t vectorSize>
inline ReturnValue_t LocalPoolVector<T, vectorSize>::read(MutexIF::TimeoutType timeoutType,
                                                          uint32_t timeoutMs) {
  MutexGuard(LocalDpManagerAttorney::getMutexHandle(*hkManager), timeoutType, timeoutMs);
  return readWithoutLock();
}
template <typename T, uint16_t vectorSize>
inline ReturnValue_t LocalPoolVector<T, vectorSize>::readWithoutLock() {
  if (readWriteMode == pool_rwm_t::VAR_WRITE) {
    object_id_t targetObjectId = hkManager->getCreatorObjectId();
    reportReadCommitError("LocalPoolVector", PoolVariableIF::INVALID_READ_WRITE_MODE, true,
                          targetObjectId, localPoolId);
    return PoolVariableIF::INVALID_READ_WRITE_MODE;
  }

  PoolEntry<T>* poolEntry = nullptr;
  ReturnValue_t result =
      LocalDpManagerAttorney::fetchPoolEntry(*hkManager, localPoolId, &poolEntry);
  memset(this->value, 0, vectorSize * sizeof(T));

  if (result != returnvalue::OK) {
    object_id_t targetObjectId = hkManager->getCreatorObjectId();
    reportReadCommitError("LocalPoolVector", result, true, targetObjectId, localPoolId);
    return result;
  }
  std::memcpy(this->value, poolEntry->getDataPtr(), poolEntry->getByteSize());
  this->valid = poolEntry->getValid();
  return returnvalue::OK;
}

template <typename T, uint16_t vectorSize>
inline ReturnValue_t LocalPoolVector<T, vectorSize>::commit(bool valid,
                                                            MutexIF::TimeoutType timeoutType,
                                                            uint32_t timeoutMs) {
  this->setValid(valid);
  return commit(timeoutType, timeoutMs);
}

template <typename T, uint16_t vectorSize>
inline ReturnValue_t LocalPoolVector<T, vectorSize>::commit(MutexIF::TimeoutType timeoutType,
                                                            uint32_t timeoutMs) {
  MutexGuard(LocalDpManagerAttorney::getMutexHandle(*hkManager), timeoutType, timeoutMs);
  return commitWithoutLock();
}

template <typename T, uint16_t vectorSize>
inline ReturnValue_t LocalPoolVector<T, vectorSize>::commitWithoutLock() {
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
    object_id_t targetObjectId = hkManager->getCreatorObjectId();
    reportReadCommitError("LocalPoolVector", result, false, targetObjectId, localPoolId);
    return result;
  }
  std::memcpy(poolEntry->getDataPtr(), this->value, poolEntry->getByteSize());
  poolEntry->setValid(this->valid);
  return returnvalue::OK;
}

template <typename T, uint16_t vectorSize>
inline T& LocalPoolVector<T, vectorSize>::operator[](size_t i) {
  if (i < vectorSize) {
    return value[i];
  }
  // If this happens, I have to set some value. I consider this
  // a configuration error, but I wont exit here.
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::warning << "LocalPoolVector: Invalid index. Setting or returning"
                  " last value!"
               << std::endl;
#else
  sif::printWarning(
      "LocalPoolVector: Invalid index. Setting or returning"
      " last value!\n");
#endif
  return value[vectorSize - 1];
}

template <typename T, uint16_t vectorSize>
inline const T& LocalPoolVector<T, vectorSize>::operator[](size_t i) const {
  if (i < vectorSize) {
    return value[i];
  }
  // If this happens, I have to set some value. I consider this
  // a configuration error, but I wont exit here.
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::warning << "LocalPoolVector: Invalid index. Setting or returning"
                  " last value!"
               << std::endl;
#else
  sif::printWarning(
      "LocalPoolVector: Invalid index. Setting or returning"
      " last value!\n");
#endif
  return value[vectorSize - 1];
}

template <typename T, uint16_t vectorSize>
inline ReturnValue_t LocalPoolVector<T, vectorSize>::serialize(
    uint8_t** buffer, size_t* size, size_t maxSize,
    SerializeIF::Endianness streamEndianness) const {
  ReturnValue_t result = returnvalue::FAILED;
  for (uint16_t i = 0; i < vectorSize; i++) {
    result = SerializeAdapter::serialize(&(value[i]), buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      break;
    }
  }
  return result;
}

template <typename T, uint16_t vectorSize>
inline size_t LocalPoolVector<T, vectorSize>::getSerializedSize() const {
  return vectorSize * SerializeAdapter::getSerializedSize(value);
}

template <typename T, uint16_t vectorSize>
inline ReturnValue_t LocalPoolVector<T, vectorSize>::deSerialize(
    const uint8_t** buffer, size_t* size, SerializeIF::Endianness streamEndianness) {
  ReturnValue_t result = returnvalue::FAILED;
  for (uint16_t i = 0; i < vectorSize; i++) {
    result = SerializeAdapter::deSerialize(&(value[i]), buffer, size, streamEndianness);
    if (result != returnvalue::OK) {
      break;
    }
  }
  return result;
}

#if FSFW_CPP_OSTREAM_ENABLED == 1
template <typename T, uint16_t vectorSize>
inline std::ostream& operator<<(std::ostream& out, const LocalPoolVector<T, vectorSize>& var) {
  out << "Vector: [";
  for (int i = 0; i < vectorSize; i++) {
    out << var.value[i];
    if (i < vectorSize - 1) {
      out << ", ";
    }
  }
  out << "]";
  return out;
}
#endif

#endif /* FSFW_DATAPOOLLOCAL_LOCALPOOLVECTOR_TPP_ */

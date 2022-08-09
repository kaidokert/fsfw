#include "StorageManagerMock.h"

ReturnValue_t StorageManagerMock::addData(store_address_t *storageId, const uint8_t *data,
                                          size_t size, bool ignoreFault) {
  if (nextAddDataCallFails.first) {
    return nextAddDataCallFails.second;
  }
  return LocalPool::addData(storageId, data, size, ignoreFault);
}
ReturnValue_t StorageManagerMock::deleteData(store_address_t packet_id) {
  if (nextDeleteDataCallFails.first) {
    return nextDeleteDataCallFails.second;
  }
  return LocalPool::deleteData(packet_id);
}

ReturnValue_t StorageManagerMock::deleteData(uint8_t *buffer, size_t size,
                                             store_address_t *storeId) {
  if (nextDeleteDataCallFails.first) {
    return nextDeleteDataCallFails.second;
  }
  return LocalPool::deleteData(buffer, size, storeId);
}

ReturnValue_t StorageManagerMock::getData(store_address_t packet_id, const uint8_t **packet_ptr,
                                          size_t *size) {
  return LocalPool::getData(packet_id, packet_ptr, size);
}

ReturnValue_t StorageManagerMock::modifyData(store_address_t packet_id, uint8_t **packet_ptr,
                                             size_t *size) {
  if (nextModifyDataCallFails.first) {
    return nextModifyDataCallFails.second;
  }
  return LocalPool::modifyData(packet_id, packet_ptr, size);
}

ReturnValue_t StorageManagerMock::getFreeElement(store_address_t *storageId, size_t size,
                                                 uint8_t **p_data, bool ignoreFault) {
  if (nextFreeElementCallFails.first) {
    return nextFreeElementCallFails.second;
  }
  return LocalPool::getFreeElement(storageId, size, p_data, ignoreFault);
}

bool StorageManagerMock::hasDataAtId(store_address_t storeId) const {
  return LocalPool::hasDataAtId(storeId);
}
void StorageManagerMock::clearStore() { return LocalPool::clearStore(); }

void StorageManagerMock::clearSubPool(uint8_t poolIndex) {
  return LocalPool::clearSubPool(poolIndex);
}

void StorageManagerMock::getFillCount(uint8_t *buffer, uint8_t *bytesWritten) {
  return LocalPool::getFillCount(buffer, bytesWritten);
}

size_t StorageManagerMock::getTotalSize(size_t *additionalSize) {
  return LocalPool::getTotalSize(additionalSize);
}

StorageManagerIF::max_subpools_t StorageManagerMock::getNumberOfSubPools() const {
  return LocalPool::getNumberOfSubPools();
}

void StorageManagerMock::reset() {
  clearStore();
  nextAddDataCallFails.first = false;
  nextAddDataCallFails.second = result::OK;
  nextModifyDataCallFails.first = false;
  nextModifyDataCallFails.second = result::OK;
  nextDeleteDataCallFails.first = false;
  nextDeleteDataCallFails.second = result::OK;
  nextFreeElementCallFails.first = false;
  nextFreeElementCallFails.second = result::OK;
}

StorageManagerMock::StorageManagerMock(object_id_t setObjectId,
                                       const LocalPool::LocalPoolConfig &poolConfig)
    : LocalPool(setObjectId, poolConfig) {}

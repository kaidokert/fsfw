#include "StorageManagerMock.h"

ReturnValue_t StorageManagerMock::addData(store_address_t *storageId, const uint8_t *data,
                                          size_t size, bool ignoreFault) {
  return 0;
}
ReturnValue_t StorageManagerMock::deleteData(store_address_t packet_id) { return 0; }

ReturnValue_t StorageManagerMock::deleteData(uint8_t *buffer, size_t size,
                                             store_address_t *storeId) {
  return 0;
}

ReturnValue_t StorageManagerMock::getData(store_address_t packet_id, const uint8_t **packet_ptr,
                                          size_t *size) {
  return 0;
}

ReturnValue_t StorageManagerMock::modifyData(store_address_t packet_id, uint8_t **packet_ptr,
                                             size_t *size) {
  return 0;
}
ReturnValue_t StorageManagerMock::getFreeElement(store_address_t *storageId, size_t size,
                                                 uint8_t **p_data, bool ignoreFault) {
  return 0;
}
bool StorageManagerMock::hasDataAtId(store_address_t storeId) const { return false; }
void StorageManagerMock::clearStore() {}
void StorageManagerMock::clearSubPool(uint8_t poolIndex) {}
void StorageManagerMock::getFillCount(uint8_t *buffer, uint8_t *bytesWritten) {}
size_t StorageManagerMock::getTotalSize(size_t *additionalSize) { return 0; }
StorageManagerIF::max_subpools_t StorageManagerMock::getNumberOfSubPools() const { return 0; }

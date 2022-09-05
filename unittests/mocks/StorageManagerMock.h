#ifndef FSFW_TESTS_STORAGEMANAGERMOCK_H
#define FSFW_TESTS_STORAGEMANAGERMOCK_H

#include "fsfw/storagemanager/LocalPool.h"
#include "fsfw/storagemanager/StorageManagerIF.h"

class StorageManagerMock : public LocalPool {
 public:
  StorageManagerMock(object_id_t setObjectId, const LocalPoolConfig &poolConfig);

  ReturnValue_t addData(store_address_t *storageId, const uint8_t *data, size_t size,
                        bool ignoreFault) override;
  ReturnValue_t deleteData(store_address_t packet_id) override;
  ReturnValue_t deleteData(uint8_t *buffer, size_t size, store_address_t *storeId) override;
  ReturnValue_t getData(store_address_t packet_id, const uint8_t **packet_ptr,
                        size_t *size) override;
  ReturnValue_t modifyData(store_address_t packet_id, uint8_t **packet_ptr, size_t *size) override;
  ReturnValue_t getFreeElement(store_address_t *storageId, size_t size, uint8_t **p_data,
                               bool ignoreFault) override;
  [[nodiscard]] bool hasDataAtId(store_address_t storeId) const override;
  void clearStore() override;
  void clearSubPool(uint8_t poolIndex) override;
  void getFillCount(uint8_t *buffer, uint8_t *bytesWritten) override;
  size_t getTotalSize(size_t *additionalSize) override;
  [[nodiscard]] max_subpools_t getNumberOfSubPools() const override;

  std::pair<bool, ReturnValue_t> nextAddDataCallFails;
  /**
   * This can be used to make both the modify and get API call fail. This is because generally,
   * the pool implementation get functions will use the modify API internally.
   */
  std::pair<bool, ReturnValue_t> nextModifyDataCallFails;
  std::pair<bool, ReturnValue_t> nextDeleteDataCallFails;
  std::pair<bool, ReturnValue_t> nextFreeElementCallFails;

  using LocalPool::getFreeElement;

  void reset();
};
#endif  // FSFW_TESTS_STORAGEMANAGERMOCK_H

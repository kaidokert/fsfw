#ifndef FSFW_STORAGEMANAGER_STORAGEACCESSOR_H_
#define FSFW_STORAGEMANAGER_STORAGEACCESSOR_H_

#include "ConstStorageAccessor.h"

class StorageManagerIF;

/**
 * @brief   Child class for modifyable data. Also has a normal pointer member.
 */
class StorageAccessor : public ConstStorageAccessor {
  //! StorageManager classes have exclusive access to private variables.
  friend class PoolManager;
  friend class LocalPool;
  friend class StorageManagerIF;

 public:
  explicit StorageAccessor(store_address_t storeId);
  StorageAccessor(store_address_t storeId, StorageManagerIF* store);

  /**
   * @brief	Move ctor and move assignment allow returning accessors as
   * 			a returnvalue. They prevent resource being freed prematurely.
   * See: https://github.com/MicrosoftDocs/cpp-docs/blob/master/docs/cpp/
   * 		move-constructors-and-move-assignment-operators-cpp.md
   * @param
   * @return
   */
  StorageAccessor& operator=(StorageAccessor&&) noexcept;
  StorageAccessor(StorageAccessor&&) noexcept;

  ReturnValue_t write(uint8_t* data, size_t size, uint16_t offset = 0);
  uint8_t* data();
  ReturnValue_t getDataCopy(uint8_t* pointer, size_t maxSize) override;

 private:
  //! Non-const pointer for modifyable data.
  uint8_t* dataPointer = nullptr;
  //! For modifyable data, the const pointer is assigned to the normal
  //! pointer by the pool manager so both access functions can be used safely
  void assignConstPointer();
};

#endif /* FSFW_STORAGEMANAGER_STORAGEACCESSOR_H_ */

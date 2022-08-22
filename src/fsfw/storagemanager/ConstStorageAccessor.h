#ifndef FSFW_STORAGEMANAGER_CONSTSTORAGEACCESSOR_H_
#define FSFW_STORAGEMANAGER_CONSTSTORAGEACCESSOR_H_

#include <cstddef>

#include "../returnvalues/returnvalue.h"
#include "storeAddress.h"

class StorageManagerIF;

/**
 * @brief 	Helper classes to facilitate safe access to storages which is also
 * 			conforming to RAII principles
 * @details
 * Accessor class which can be returned by pool manager or passed and set by
 * pool managers to have safe access to the pool resources.
 *
 * These helper can be used together with the StorageManager classes to manage
 * access to a storage. It can take care of thread-safety while also providing
 * mechanisms to automatically clear storage data.
 */
class ConstStorageAccessor {
  //! StorageManager classes have exclusive access to private variables.
  friend class PoolManager;
  friend class LocalPool;
  friend class StorageManagerIF;

 public:
  /**
   * @brief	Simple constructor which takes the store ID of the storage
   * 			entry to access.
   * @param storeId
   */
  explicit ConstStorageAccessor(store_address_t storeId);
  ConstStorageAccessor(store_address_t storeId, StorageManagerIF* store);

  /**
   * @brief 	The destructor in default configuration takes care of
   * 			deleting the accessed pool entry and unlocking the mutex
   */
  virtual ~ConstStorageAccessor();

  /**
   * @brief	Returns a pointer to the read-only data
   * @return
   */
  [[nodiscard]] const uint8_t* data() const;

  /**
   * @brief	Copies the read-only data to the supplied pointer
   * @param pointer
   */
  virtual ReturnValue_t getDataCopy(uint8_t* pointer, size_t maxSize);

  /**
   * @brief   Calling this will prevent the Accessor from deleting the data
   *          when the destructor is called.
   */
  void release();

  /**
   * Get the size of the data
   * @return
   */
  [[nodiscard]] size_t size() const;

  /**
   * Get the storage ID.
   * @return
   */
  [[nodiscard]] store_address_t getId() const;

  void print() const;

  /**
   * @brief	Move ctor and move assignment allow returning accessors as
   * 			a returnvalue. They prevent resource being free prematurely.
   * Refer to: https://github.com/MicrosoftDocs/cpp-docs/blob/master/docs/cpp/
   * move-constructors-and-move-assignment-operators-cpp.md
   * @param
   * @return
   */
  ConstStorageAccessor& operator=(ConstStorageAccessor&&) noexcept;
  ConstStorageAccessor(ConstStorageAccessor&&) noexcept;

  //! The copy ctor and copy assignemnt should be deleted implicitely
  //! according to https://foonathan.net/2019/02/special-member-functions/
  //! but I still deleted them to make it more explicit. (remember rule of 5).
  ConstStorageAccessor& operator=(const ConstStorageAccessor&) = delete;
  ConstStorageAccessor(const ConstStorageAccessor&) = delete;

 protected:
  const uint8_t* constDataPointer = nullptr;
  store_address_t storeId;
  size_t size_ = 0;
  //! Managing pool, has to assign itself.
  StorageManagerIF* store = nullptr;
  bool deleteData = true;

  enum class AccessState { UNINIT, ASSIGNED };
  //! Internal state for safety reasons.
  AccessState internalState = AccessState::UNINIT;
  /**
   * Used by the pool manager instances to assign themselves to the
   * accessor. This is necessary to delete the data when the acessor
   * exits the scope ! The internal state will be considered read
   * when this function is called, so take care all data is set properly as
   * well.
   * @param
   */
  void assignStore(StorageManagerIF*);
};

#endif /* FSFW_STORAGEMANAGER_CONSTSTORAGEACCESSOR_H_ */

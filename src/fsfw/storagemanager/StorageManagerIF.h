#ifndef FSFW_STORAGEMANAGER_STORAGEMANAGERIF_H_
#define FSFW_STORAGEMANAGER_STORAGEMANAGERIF_H_

#include <cstddef>
#include <utility>

#include "../events/Event.h"
#include "../returnvalues/returnvalue.h"
#include "StorageAccessor.h"
#include "storeAddress.h"

using AccessorPair = std::pair<ReturnValue_t, StorageAccessor>;
using ConstAccessorPair = std::pair<ReturnValue_t, ConstStorageAccessor>;

/**
 * @brief	This class provides an interface for intermediate data storage.
 * @details	The Storage manager classes shall be used to store larger chunks of
 * 			data in RAM for exchange between tasks. This interface expects the
 * 			data to be stored in one consecutive block of memory, so tasks can
 * 			write directly to the destination pointer.
 * 			For interprocess communication, the stores must be locked during
 * 			insertion and deletion. If the receiving storage identifier is
 * 			passed token-like between tasks, a lock during read and write
 * 			operations is not necessary.
 * @author	Bastian Baetz
 * @date	18.09.2012
 */
class StorageManagerIF {
 public:
  using size_type = size_t;
  using max_subpools_t = uint8_t;

  static const uint8_t INTERFACE_ID =
      CLASS_ID::STORAGE_MANAGER_IF;  //!< The unique ID for return codes for this interface.
  static const ReturnValue_t DATA_TOO_LARGE = MAKE_RETURN_CODE(
      1);  //!< This return code indicates that the data to be stored is too large for the store.
  static const ReturnValue_t DATA_STORAGE_FULL =
      MAKE_RETURN_CODE(2);  //!< This return code indicates that a data storage is full.
  static const ReturnValue_t ILLEGAL_STORAGE_ID = MAKE_RETURN_CODE(
      3);  //!< This return code indicates that data was requested with an illegal storage ID.
  static const ReturnValue_t DATA_DOES_NOT_EXIST =
      MAKE_RETURN_CODE(4);  //!< This return code indicates that the requested ID was valid, but no
                            //!< data is stored there.
  static const ReturnValue_t ILLEGAL_ADDRESS = MAKE_RETURN_CODE(5);
  static const ReturnValue_t POOL_TOO_LARGE =
      MAKE_RETURN_CODE(6);  //!< Pool size too large on initialization.

  static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::OBSW;
  static const Event GET_DATA_FAILED = MAKE_EVENT(0, severity::LOW);
  static const Event STORE_DATA_FAILED = MAKE_EVENT(1, severity::LOW);

  //!< Indicates an invalid (i.e unused) storage address.
  static const uint32_t INVALID_ADDRESS = 0xFFFFFFFF;

  /**
   * @brief This is the empty virtual destructor as required for C++ interfaces.
   */
  ~StorageManagerIF() = default;
  /**
   * @brief	With addData, a free storage position is allocated and data
   * 			stored there.
   * @details	During the allocation, the StorageManager is blocked.
   * @param storageId A pointer to the storageId to retrieve.
   * @param data	The data to be stored in the StorageManager.
   * @param size	The amount of data to be stored.
   * @return	Returns @returnvalue::OK if data was added.
   * 		@returnvalue::FAILED if data could not be added, storageId is unchanged then.
   */
  virtual ReturnValue_t addData(store_address_t* storageId, const uint8_t* data, size_t size,
                                bool ignoreFault) = 0;

  virtual ReturnValue_t addData(store_address_t* storageId, const uint8_t* data, size_t size) {
    return addData(storageId, data, size, false);
  }

  /**
   * @brief	With deleteData, the storageManager frees the memory region
   * 			identified by packet_id.
   * @param packet_id	The identifier of the memory region to be freed.
   * @return	@li returnvalue::OK on success.
   * 			@li	returnvalue::FAILED if deletion did not work
   * 				(e.g. an illegal packet_id was passed).
   */
  virtual ReturnValue_t deleteData(store_address_t packet_id) = 0;
  /**
   * @brief	Another deleteData which uses the pointer and size of the
   * 			stored data to delete the content.
   * @param buffer	Pointer to the data.
   * @param size		Size of data to be stored.
   * @param storeId	Store id of the deleted element (optional)
   * @return	@li returnvalue::OK on success.
   * 			@li	failure code if deletion did not work
   */
  virtual ReturnValue_t deleteData(uint8_t* buffer, size_t size, store_address_t* storeId) = 0;
  virtual ReturnValue_t deleteData(uint8_t* buffer, size_t size) {
    return deleteData(buffer, size, nullptr);
  }
  /**
   * @brief 	Access the data by supplying a store ID.
   * @details
   * A pair consisting of the retrieval result and an instance of a
   * ConstStorageAccessor class is returned
   * @param storeId
   * @return Pair of return value and a ConstStorageAccessor instance
   */
  virtual ConstAccessorPair getData(store_address_t storeId) {
    uint8_t* tempData = nullptr;
    ConstStorageAccessor constAccessor(storeId, this);
    ReturnValue_t status = modifyData(storeId, &tempData, &constAccessor.size_);
    constAccessor.constDataPointer = tempData;
    return {status, std::move(constAccessor)};
  }

  /**
   * @brief 	Access the data by supplying a store ID and a helper
   * 			instance
   * @param storeId
   * @param constAccessor Wrapper function to access store data.
   * @return
   */
  virtual ReturnValue_t getData(store_address_t storeId, ConstStorageAccessor& accessor) {
    uint8_t* tempData = nullptr;
    ReturnValue_t status = modifyData(storeId, &tempData, &accessor.size_);
    accessor.assignStore(this);
    accessor.constDataPointer = tempData;
    return status;
  }

  /**
   * @brief	getData returns an address to data and the size of the data
   * 			for a given packet_id.
   * @param packet_id		The id of the data to be returned
   * @param packet_ptr	The passed pointer address is set to the the memory
   * 						position
   * @param size			The exact size of the stored data is returned here.
   * @return	@returnvalue::OK on success.
   * 		@returnvalue::FAILED if fetching data did not work
   * 				(e.g. an illegal packet_id was passed).
   */
  virtual ReturnValue_t getData(store_address_t packet_id, const uint8_t** packet_ptr,
                                size_t* size) = 0;

  /**
   * Modify data by supplying a store ID
   * @param storeId
   * @return Pair of return value and StorageAccessor helper
   */
  virtual AccessorPair modifyData(store_address_t storeId) {
    StorageAccessor accessor(storeId, this);
    ReturnValue_t status = modifyData(storeId, &accessor.dataPointer, &accessor.size_);
    accessor.assignConstPointer();
    return {status, std::move(accessor)};
  }

  /**
   * Modify data by supplying a store ID and a StorageAccessor helper instance.
   * @param storeId
   * @param accessor Helper class to access the modifiable data.
   * @return
   */
  virtual ReturnValue_t modifyData(store_address_t storeId, StorageAccessor& accessor) {
    accessor.assignStore(this);
    ReturnValue_t status = modifyData(storeId, &accessor.dataPointer, &accessor.size_);
    accessor.assignConstPointer();
    return status;
  }

  /**
   * Get pointer and size of modifiable data by supplying the storeId
   * @param packet_id
   * @param packet_ptr [out] Pointer to pointer of data to set
   * @param size [out] Pointer to size to set
   * @return
   */
  virtual ReturnValue_t modifyData(store_address_t packet_id, uint8_t** packet_ptr,
                                   size_t* size) = 0;
  /**
   * This method reserves an element of @c size.
   *
   * It returns the packet id of this element as well as a direct pointer to the
   * data of the element. It must be assured that exactly @c size data is
   * written to p_data!
   * @param storageId A pointer to the storageId to retrieve.
   * @param size		The size of the space to be reserved.
   * @param dataPtr	A pointer to the element data is returned here.
   * @return	Returns @returnvalue::OK if data was added.
   * 		@returnvalue::FAILED if data could not be added, storageId is unchanged then.
   */
  virtual ReturnValue_t getFreeElement(store_address_t* storageId, size_t size, uint8_t** dataPtr,
                                       bool ignoreFault) = 0;

  virtual ReturnValue_t getFreeElement(store_address_t* storageId, size_t size, uint8_t** dataPtr) {
    return getFreeElement(storageId, size, dataPtr, false);
  }

  [[nodiscard]] virtual bool hasDataAtId(store_address_t storeId) const = 0;

  /**
   * Clears the whole store.
   * Use with care!
   */
  virtual void clearStore() = 0;

  /**
   * Clears a pool in the store with the given pool index. Use with care!
   * @param pageIndex
   */
  virtual void clearSubPool(uint8_t poolIndex) = 0;

  /**
   * Get the fill count of the pool. Each character inside the provided
   * buffer will be assigned to a rounded percentage fill count for each
   * page. The last written byte (at the index bytesWritten - 1)
   * will contain the total fill count of the pool as a mean of the
   * percentages of single pages.
   * @param buffer
   * @param maxSize
   */
  virtual void getFillCount(uint8_t* buffer, uint8_t* bytesWritten) = 0;

  virtual size_t getTotalSize(size_t* additionalSize) = 0;

  /**
   * Get number of pools.
   * @return
   */
  [[nodiscard]] virtual max_subpools_t getNumberOfSubPools() const = 0;
};

#endif /* FSFW_STORAGEMANAGER_STORAGEMANAGERIF_H_ */

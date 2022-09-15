#ifndef FSFW_STORAGEMANAGER_LOCALPOOL_H_
#define FSFW_STORAGEMANAGER_LOCALPOOL_H_

#include <limits>
#include <set>
#include <utility>
#include <vector>

#include "fsfw/internalerror/InternalErrorReporterIF.h"
#include "fsfw/objectmanager/ObjectManagerIF.h"
#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/serviceinterface/ServiceInterfaceStream.h"
#include "fsfw/storagemanager/StorageAccessor.h"
#include "fsfw/storagemanager/StorageManagerIF.h"

/**
 * @brief   The LocalPool class provides an intermediate data storage with
 *          a fixed pool size policy.
 * @details
 * The class implements the StorageManagerIF interface. While the total number
 * of pools is fixed, the element sizes in one pool and the number of pool
 * elements per pool are set on construction. The full amount of memory is
 * allocated on construction.
 * The overhead is 4 byte per pool element to store the size information of
 * each stored element. To maintain an "empty" information, the pool size is
 * limited to 0xFFFF-1 bytes.
 * It is possible to store empty packets in the pool.
 * The local pool is NOT thread-safe.
 */
class LocalPool : public SystemObject, public StorageManagerIF {
 public:
  using pool_elem_size_t = size_type;
  using n_pool_elem_t = uint16_t;
  using LocalPoolCfgPair = std::pair<n_pool_elem_t, pool_elem_size_t>;

  // The configuration needs to be provided with the pool sizes ascending
  // but the number of pool elements as the first value is more intuitive.
  // Therefore, a custom comparator was provided.
  struct LocalPoolConfigCmp {
    bool operator()(const LocalPoolCfgPair& a, const LocalPoolCfgPair& b) const {
      if (a.second < b.second) {
        return true;
      } else if (a.second > b.second) {
        return false;
      } else {
        if (a.first < b.first) {
          return true;
        } else {
          return false;
        }
      }
    }
  };
  using LocalPoolConfig = std::multiset<LocalPoolCfgPair, LocalPoolConfigCmp>;

  /**
   * @brief   This is the default constructor for a pool manager instance.
   * @details
   * The pool is configured by passing a set of pairs into the constructor.
   * The first value of that pair determines the number of one elements on
   * the respective page of the pool while the second value determines how
   * many elements with that size are created on that page.
   * All regions are to zero on start up.
   * @param setObjectId   The object identifier to be set. This allows for
   *                      multiple instances of LocalPool in the system.
   * @param poolConfig
   * This is a set of pairs to configure the number of pages in the pool,
   * the size of an element on a page, the number of elements on a page
   * and the total size of the pool at once while also implicitely
   * sorting the pairs in the right order.
   * @param registered
   * Determines whether the pool is registered in the object manager or not.
   * @param spillsToHigherPools A variable to determine whether
   * higher n pools are used if the store is full.
   */
  LocalPool(object_id_t setObjectId, const LocalPoolConfig& poolConfig, bool registered = false,
            bool spillsToHigherPools = false);

  void setToSpillToHigherPools(bool enable);

  /**
   * @brief	In the LocalPool's destructor all allocated memory is freed.
   */
  ~LocalPool() override;

  /**
   * Documentation: See StorageManagerIF.h
   */
  ReturnValue_t addData(store_address_t* storeId, const uint8_t* data, size_t size,
                        bool ignoreFault) override;
  ReturnValue_t addData(store_address_t* storeId, const uint8_t* data, size_t size) override;

  ReturnValue_t getFreeElement(store_address_t* storeId, size_t size, uint8_t** pData) override;
  ReturnValue_t getFreeElement(store_address_t* storeId, size_t size, uint8_t** pData,
                               bool ignoreFault) override;

  ConstAccessorPair getData(store_address_t storeId) override;
  ReturnValue_t getData(store_address_t storeId, ConstStorageAccessor& accessor) override;
  ReturnValue_t getData(store_address_t storeId, const uint8_t** packet_ptr, size_t* size) override;

  AccessorPair modifyData(store_address_t storeId) override;
  ReturnValue_t modifyData(store_address_t storeId, uint8_t** packet_ptr, size_t* size) override;
  ReturnValue_t modifyData(store_address_t storeId, StorageAccessor& accessor) override;

  ReturnValue_t deleteData(store_address_t storeId) override;
  ReturnValue_t deleteData(uint8_t* ptr, size_t size, store_address_t* storeId) override;

  /**
   * Get the total size of allocated memory for pool data.
   * There is an additional overhead of the sizes of elements which will
   * be assigned to additionalSize
   * @return
   */
  size_t getTotalSize(size_t* additionalSize) override;

  /**
   * Get the fill count of the pool. Each character inside the provided
   * buffer will be assigned to a rounded percentage fill count for each
   * page. The last written byte (at the index bytesWritten - 1)
   * will contain the total fill count of the pool as a mean of the
   * percentages of single pages.
   * @param buffer
   * @param maxSize
   */
  void getFillCount(uint8_t* buffer, uint8_t* bytesWritten) override;

  void clearStore() override;
  void clearSubPool(max_subpools_t poolIndex) override;

  ReturnValue_t initialize() override;

  /**
   * Get number sub pools. Each pool has pages with a specific bucket size.
   * @return
   */
  [[nodiscard]] max_subpools_t getNumberOfSubPools() const override;
  [[nodiscard]] bool hasDataAtId(store_address_t storeId) const override;

 protected:
  /**
   * With this helper method, a free element of @c size is reserved.
   * @param size	The minimum packet size that shall be reserved.
   * @param[out] address Storage ID of the reserved data.
   * @return	- returnvalue::OK on success,
   * 			- the return codes of #getPoolIndex or #findEmpty otherwise.
   */
  virtual ReturnValue_t reserveSpace(size_t size, store_address_t* address, bool ignoreFault);

 private:
  /**
   * @brief   This definition generally sets the number of
   *          different sized pools. It is derived from the number of pairs
   *          inside the LocalPoolConfig set on object creation.
   * @details
   * This must be less than the maximum number of pools (currently 0xff).
   */
  const max_subpools_t NUMBER_OF_SUBPOOLS;

  /**
   * Indicates that this element is free.
   * This value limits the maximum size of a pool.
   * Change to larger data type if increase is required.
   */
  static const size_type STORAGE_FREE = std::numeric_limits<size_type>::max();
  /**
   * @brief	In this array, the element sizes of each pool is stored.
   * @details	The sizes are maintained for internal pool management. The sizes
   * 			must be set in ascending order on construction.
   */
  std::vector<size_type> elementSizes = std::vector<size_type>(NUMBER_OF_SUBPOOLS);
  /**
   * @brief	n_elements stores the number of elements per pool.
   * @details	These numbers are maintained for internal pool management.
   */
  std::vector<uint16_t> numberOfElements = std::vector<uint16_t>(NUMBER_OF_SUBPOOLS);
  /**
   * @brief	store represents the actual memory pool.
   * @details	It is an array of pointers to memory, which was allocated with
   * 			a @c new call on construction.
   */
  std::vector<std::vector<uint8_t>> store = std::vector<std::vector<uint8_t>>(NUMBER_OF_SUBPOOLS);

  /**
   * @brief	The size_list attribute stores the size values of every pool element.
   * @details	As the number of elements is determined on construction, the size list
   * 			is also dynamically allocated there.
   */
  std::vector<std::vector<size_type>> sizeLists =
      std::vector<std::vector<size_type>>(NUMBER_OF_SUBPOOLS);

  //! A variable to determine whether higher n pools are used if
  //! the store is full.
  bool spillsToHigherPools = false;
  /**
   * @brief	This method safely stores the given data in the given packet_id.
   * @details	It also sets the size in size_list. The method does not perform
   * 			any range checks, these are done in advance.
   * @param packet_id	The storage identifier in which the data shall be stored.
   * @param data		The data to be stored.
   * @param size		The size of the data to be stored.
   */
  void write(store_address_t packetId, const uint8_t* data, size_t size);
  /**
   * @brief	A helper method to read the element size of a certain pool.
   * @param pool_index	The pool in which to look.
   * @return	Returns the size of an element or 0.
   */
  size_type getSubpoolElementSize(max_subpools_t subpoolIndex);

  /**
   * @brief	This helper method looks up a fitting pool for a given size.
   * @details	The pools are looked up in ascending order, so the first that
   * 			fits is used.
   * @param packet_size		The size of the data to be stored.
   * @param[out] poolIndex	The fitting pool index found.
   * @return	- @c returnvalue::OK on success,
   * 			- @c DATA_TOO_LARGE otherwise.
   */
  ReturnValue_t getSubPoolIndex(size_t packetSize, uint16_t* subpoolIndex,
                                uint16_t startAtIndex = 0);
  /**
   * @brief	This helper method calculates the true array position in store
   * 			of a given packet id.
   * @details	The method does not perform any range checks, these are done in
   * 			advance.
   * @param packet_id	The packet id to look up.
   * @return	Returns the position of the data in store.
   */
  size_type getRawPosition(store_address_t storeId);
  /**
   * @brief	This is a helper method to find an empty element in a given pool.
   * @details	The method searches size_list for the first empty element, so
   * 			duration grows with the fill level of the pool.
   * @param pool_index	The pool in which the search is performed.
   * @param[out] element	The first found element in the pool.
   * @return	- returnvalue::OK on success,
   * 			- #DATA_STORAGE_FULL if the store is full
   */
  ReturnValue_t findEmpty(n_pool_elem_t poolIndex, uint16_t* element);

  InternalErrorReporterIF* internalErrorReporter = nullptr;
};

#endif /* FSFW_STORAGEMANAGER_LOCALPOOL_H_ */

#ifndef FRAMEWORK_STORAGEMANAGER_LOCALPOOL_H_
#define FRAMEWORK_STORAGEMANAGER_LOCALPOOL_H_

/**
 *	@file	LocalPool
 *
 *  @date	02.02.2012
 *	@author	Bastian Baetz
 *
 *	@brief	This file contains the definition of the LocalPool class.
 */

#include <framework/objectmanager/SystemObject.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/storagemanager/StorageManagerIF.h>
#include <framework/objectmanager/ObjectManagerIF.h>
#include <framework/internalError/InternalErrorReporterIF.h>
#include <string.h>

/**
 * @brief	The LocalPool class provides an intermediate data storage with
 * 			a fixed pool size policy.
 * \details	The class implements the StorageManagerIF interface. While the
 * 			total number of pools is fixed, the element sizes in one pool and
 * 			the number of pool elements per pool are set on construction.
 * 			The full amount of memory is allocated on construction.
 * 			The overhead is 4 byte per pool element to store the size
 * 			information of each stored element.
 * 			To maintain an "empty" information, the pool size is limited to
 * 			0xFFFF-1 bytes.
 * 			It is possible to store empty packets in the pool.
 * 			The local pool is NOT thread-safe.
 */

template<uint8_t NUMBER_OF_POOLS = 5>
class LocalPool: public SystemObject, public StorageManagerIF {
public:
	/**
	 * @brief	This definition generally sets the number of different sized pools.
	 * @details This must be less than the maximum number of pools (currently 0xff).
	 */
//	static const uint32_t NUMBER_OF_POOLS;
private:
	/**
	 * Indicates that this element is free.
	 * This value limits the maximum size of a pool. Change to larger data type if increase is required.
	 */
	static const uint32_t STORAGE_FREE = 0xFFFFFFFF;
	/**
	 * @brief	In this array, the element sizes of each pool is stored.
	 * @details	The sizes are maintained for internal pool management. The sizes
	 * 			must be set in ascending order on construction.
	 */
	uint32_t element_sizes[NUMBER_OF_POOLS];
	/**
	 * @brief	n_elements stores the number of elements per pool.
	 * @details	These numbers are maintained for internal pool management.
	 */
	uint16_t n_elements[NUMBER_OF_POOLS];
	/**
	 * @brief	store represents the actual memory pool.
	 * @details	It is an array of pointers to memory, which was allocated with
	 * 			a \c new call on construction.
	 */
	uint8_t* store[NUMBER_OF_POOLS];
	/**
	 * @brief	The size_list attribute stores the size values of every pool element.
	 * @details	As the number of elements is determined on construction, the size list
	 * 			is also dynamically allocated there.
	 */
	uint32_t* size_list[NUMBER_OF_POOLS];
	bool spillsToHigherPools; //!< A variable to determine whether higher n pools are used if the store is full.
	/**
	 * @brief	This method safely stores the given data in the given packet_id.
	 * @details	It also sets the size in size_list. The method does not perform
	 * 			any range checks, these are done in advance.
	 * @param packet_id	The storage identifier in which the data shall be stored.
	 * @param data		The data to be stored.
	 * @param size		The size of the data to be stored.
	 */
	void write(store_address_t packet_id, const uint8_t* data, uint32_t size);
	/**
	 * @brief	A helper method to read the element size of a certain pool.
	 * @param pool_index	The pool in which to look.
	 * @return	Returns the size of an element or 0.
	 */
	uint32_t getPageSize(uint16_t pool_index);
	/**
	 * @brief	This helper method looks up a fitting pool for a given size.
	 * @details	The pools are looked up in ascending order, so the first that
	 * 			fits is used.
	 * @param packet_size	The size of the data to be stored.
	 * @return	Returns the pool that fits or StorageManagerIF::INVALID_ADDRESS.
	 */
	/**
	 * @brief	This helper method looks up a fitting pool for a given size.
	 * @details	The pools are looked up in ascending order, so the first that
	 * 			fits is used.
	 * @param packet_size		The size of the data to be stored.
	 * @param[out] poolIndex	The fitting pool index found.
	 * @return	- #RETURN_OK on success,
	 * 			- #DATA_TOO_LARGE otherwise.
	 */
	ReturnValue_t getPoolIndex(uint32_t packet_size, uint16_t* poolIndex, uint16_t startAtIndex = 0);
	/**
	 * @brief	This helper method calculates the true array position in store
	 * 			of a given packet id.
	 * @details	The method does not perform any range checks, these are done in
	 * 			advance.
	 * @param packet_id	The packet id to look up.
	 * @return	Returns the position of the data in store.
	 */
	uint32_t getRawPosition(store_address_t packet_id);
	/**
	 * @brief	This is a helper method to find an empty element in a given pool.
	 * @details	The method searches size_list for the first empty element, so
	 * 			duration grows with the fill level of the pool.
	 * @param pool_index	The pool in which the search is performed.
	 * @param[out] element	The first found element in the pool.
	 * @return	- #RETURN_OK on success,
	 * 			- #DATA_STORAGE_FULL if the store is full
	 */
	ReturnValue_t findEmpty(uint16_t pool_index, uint16_t* element);
protected:
	/**
	 * With this helper method, a free element of \c size is reserved.
	 * @param size	The minimum packet size that shall be reserved.
	 * @param[out] address Storage ID of the reserved data.
	 * @return	- #RETURN_OK on success,
	 * 			- the return codes of #getPoolIndex or #findEmpty otherwise.
	 */
	virtual ReturnValue_t reserveSpace(const uint32_t size, store_address_t* address, bool ignoreFault);

	InternalErrorReporterIF *internalErrorReporter;
public:
	/**
	 * @brief	This is the default constructor for a pool manager instance.
	 * @details	By passing two arrays of size NUMBER_OF_POOLS, the constructor
	 * 			allocates memory (with \c new) for store and size_list. These
	 * 			regions are all set to zero on start up.
	 * @param setObjectId	The object identifier to be set. This allows for
	 * 						multiple instances of LocalPool in the system.
	 * @param element_sizes	An array of size NUMBER_OF_POOLS in which the size
	 * 						of a single element in each pool is determined.
	 * 						<b>The sizes must be provided in ascending order.
	 * 						</b>
	 * @param n_elements	An array of size NUMBER_OF_POOLS in which the
	 * 						number of elements for each pool is determined.
	 * 						The position of these values correspond to those in
	 * 						element_sizes.
	 * @param registered	Register the pool in object manager or not. Default is false (local pool).
	 * @param spillsToHigherPools
	 * 						A variable to determine whether higher n pools are used if the store is full.
	 */
	LocalPool(object_id_t setObjectId,
			const uint16_t element_sizes[NUMBER_OF_POOLS],
			const uint16_t n_elements[NUMBER_OF_POOLS],
			bool registered = false,
			bool spillsToHigherPools = false);
	/**
	 * @brief	In the LocalPool's destructor all allocated memory is freed.
	 */
	virtual ~LocalPool(void);

	/**
	 * Add data to local data pool, performs range check
	 * @param storageId [out] Store ID in which the data will be stored
	 * @param data
	 * @param size
	 * @param ignoreFault
	 * @return @c RETURN_OK if write was successful
	 */
	ReturnValue_t addData(store_address_t* storageId, const uint8_t * data,
			uint32_t size, bool ignoreFault = false);

	/**
	 * With this helper method, a free element of \c size is reserved.
	 *
	 * @param size	The minimum packet size that shall be reserved.
	 * @return	Returns the storage identifier within the storage or
	 * 			StorageManagerIF::INVALID_ADDRESS (in raw).
	 */
	ReturnValue_t getFreeElement(store_address_t* storageId,
			const uint32_t size, uint8_t** p_data, bool ignoreFault = false);

	/**
	 * Retrieve data from local pool
	 * @param packet_id
	 * @param packet_ptr
	 * @param size [out] Size of retrieved data
	 * @return @c RETURN_OK if data retrieval was successfull
	 */
	ReturnValue_t getData(store_address_t packet_id, const uint8_t** packet_ptr,
			uint32_t* size);

	/**
	 * Modify data by supplying a packet pointer and using that packet pointer
	 * to access and modify the pool entry (via *pointer call)
	 * @param packet_id Store ID of data to modify
	 * @param packet_ptr [out] pointer to the pool entry to modify
	 * @param size [out] size of pool entry
	 * @return
	 */
	ReturnValue_t modifyData(store_address_t packet_id, uint8_t** packet_ptr,
			uint32_t* size);
	virtual ReturnValue_t deleteData(store_address_t);
	virtual ReturnValue_t deleteData(uint8_t* ptr, uint32_t size,
			store_address_t* storeId = NULL);
	void clearStore();
	ReturnValue_t initialize();
};

template<uint8_t NUMBER_OF_POOLS>
inline ReturnValue_t LocalPool<NUMBER_OF_POOLS>::findEmpty(uint16_t pool_index,
		uint16_t* element) {
	ReturnValue_t status = DATA_STORAGE_FULL;
	for (uint16_t foundElement = 0; foundElement < n_elements[pool_index];
			foundElement++) {
		if (size_list[pool_index][foundElement] == STORAGE_FREE) {
			*element = foundElement;
			status = RETURN_OK;
			break;
		}
	}
	return status;
}

template<uint8_t NUMBER_OF_POOLS>
inline void LocalPool<NUMBER_OF_POOLS>::write(store_address_t packet_id,
		const uint8_t* data, uint32_t size) {
	uint8_t* ptr;
	uint32_t packet_position = getRawPosition(packet_id);

	//check size? -> Not necessary, because size is checked before calling this function.
	ptr = &store[packet_id.pool_index][packet_position];
	memcpy(ptr, data, size);
	size_list[packet_id.pool_index][packet_id.packet_index] = size;
}

//Returns page size of 0 in case store_index is illegal
template<uint8_t NUMBER_OF_POOLS>
inline uint32_t LocalPool<NUMBER_OF_POOLS>::getPageSize(uint16_t pool_index) {
	if (pool_index < NUMBER_OF_POOLS) {
		return element_sizes[pool_index];
	} else {
		return 0;
	}
}

template<uint8_t NUMBER_OF_POOLS>
inline ReturnValue_t LocalPool<NUMBER_OF_POOLS>::getPoolIndex(
		uint32_t packet_size, uint16_t* poolIndex, uint16_t startAtIndex) {
	for (uint16_t n = startAtIndex; n < NUMBER_OF_POOLS; n++) {
//		debug << "LocalPool " << getObjectId() << "::getPoolIndex: Pool: " << n << ", Element Size: " << element_sizes[n] << std::endl;
		if (element_sizes[n] >= packet_size) {
			*poolIndex = n;
			return RETURN_OK;
		}
	}
	return DATA_TOO_LARGE;
}

template<uint8_t NUMBER_OF_POOLS>
inline uint32_t LocalPool<NUMBER_OF_POOLS>::getRawPosition(
		store_address_t packet_id) {
	return packet_id.packet_index * element_sizes[packet_id.pool_index];
}

template<uint8_t NUMBER_OF_POOLS>
inline ReturnValue_t LocalPool<NUMBER_OF_POOLS>::reserveSpace(
		const uint32_t size, store_address_t* address, bool ignoreFault) {
	ReturnValue_t status = getPoolIndex(size, &address->pool_index);
	if (status != RETURN_OK) {
		error << "LocalPool( " << std::hex << getObjectId() << std::dec
				<< " )::reserveSpace: Packet too large." << std::endl;
		return status;
	}
	status = findEmpty(address->pool_index, &address->packet_index);
	while (status != RETURN_OK && spillsToHigherPools) {
		status = getPoolIndex(size, &address->pool_index, address->pool_index + 1);
		if (status != RETURN_OK) {
			//We don't find any fitting pool anymore.
			break;
		}
		status = findEmpty(address->pool_index, &address->packet_index);
	}
	if (status == RETURN_OK) {
//		if (getObjectId() == objects::IPC_STORE && address->pool_index >= 3) {
//			debug << "Reserve: Pool: " << std::dec << address->pool_index << " Index: " << address->packet_index << std::endl;
//		}

		size_list[address->pool_index][address->packet_index] = size;
	} else {
		if (!ignoreFault) {
			internalErrorReporter->storeFull();
		}
//		error << "LocalPool( " << std::hex << getObjectId() << std::dec
//				<< " )::reserveSpace: Packet store is full." << std::endl;
	}
	return status;
}

template<uint8_t NUMBER_OF_POOLS>
inline LocalPool<NUMBER_OF_POOLS>::LocalPool(object_id_t setObjectId,
		const uint16_t element_sizes[NUMBER_OF_POOLS],
		const uint16_t n_elements[NUMBER_OF_POOLS], bool registered, bool spillsToHigherPools) :
		SystemObject(setObjectId, registered), spillsToHigherPools(spillsToHigherPools), internalErrorReporter(NULL) {
	for (uint16_t n = 0; n < NUMBER_OF_POOLS; n++) {
		this->element_sizes[n] = element_sizes[n];
		this->n_elements[n] = n_elements[n];
		store[n] = new uint8_t[n_elements[n] * element_sizes[n]];
		size_list[n] = new uint32_t[n_elements[n]];
		memset(store[n], 0x00, (n_elements[n] * element_sizes[n]));
		memset(size_list[n], STORAGE_FREE, (n_elements[n] * sizeof(**size_list))); //TODO checkme
	}
}

template<uint8_t NUMBER_OF_POOLS>
inline LocalPool<NUMBER_OF_POOLS>::~LocalPool(void) {
	for (uint16_t n = 0; n < NUMBER_OF_POOLS; n++) {
		delete[] store[n];
		delete[] size_list[n];
	}
}

template<uint8_t NUMBER_OF_POOLS>
inline ReturnValue_t LocalPool<NUMBER_OF_POOLS>::addData(
		store_address_t* storageId, const uint8_t* data, uint32_t size, bool ignoreFault) {
	ReturnValue_t status = reserveSpace(size, storageId, ignoreFault);
	if (status == RETURN_OK) {
		write(*storageId, data, size);
	}
	return status;
}

template<uint8_t NUMBER_OF_POOLS>
inline ReturnValue_t LocalPool<NUMBER_OF_POOLS>::getFreeElement(
		store_address_t* storageId, const uint32_t size, uint8_t** p_data, bool ignoreFault) {
	ReturnValue_t status = reserveSpace(size, storageId, ignoreFault);
	if (status == RETURN_OK) {
		*p_data = &store[storageId->pool_index][getRawPosition(*storageId)];
	} else {
		*p_data = NULL;
	}
	return status;
}

template<uint8_t NUMBER_OF_POOLS>
inline ReturnValue_t LocalPool<NUMBER_OF_POOLS>::getData(
		store_address_t packet_id, const uint8_t** packet_ptr, uint32_t* size) {
	uint8_t* tempData = NULL;
	ReturnValue_t status = modifyData(packet_id, &tempData, size);
	*packet_ptr = tempData;
	return status;
}

template<uint8_t NUMBER_OF_POOLS>
inline ReturnValue_t LocalPool<NUMBER_OF_POOLS>::modifyData(store_address_t packet_id,
		uint8_t** packet_ptr, uint32_t* size) {
	ReturnValue_t status = RETURN_FAILED;
	if (packet_id.pool_index >= NUMBER_OF_POOLS) {
		return ILLEGAL_STORAGE_ID;
	}
	if ((packet_id.packet_index >= n_elements[packet_id.pool_index])) {
		return ILLEGAL_STORAGE_ID;
	}
	if (size_list[packet_id.pool_index][packet_id.packet_index]
			!= STORAGE_FREE) {
		uint32_t packet_position = getRawPosition(packet_id);
		*packet_ptr = &store[packet_id.pool_index][packet_position];
		*size = size_list[packet_id.pool_index][packet_id.packet_index];
		status = RETURN_OK;
	} else {
		status = DATA_DOES_NOT_EXIST;
	}
	return status;
}

template<uint8_t NUMBER_OF_POOLS>
inline ReturnValue_t LocalPool<NUMBER_OF_POOLS>::deleteData(
		store_address_t packet_id) {

//	if (getObjectId() == objects::IPC_STORE && packet_id.pool_index >= 3) {
//		debug << "Delete: Pool: " << std::dec << packet_id.pool_index << " Index: " << packet_id.packet_index << std::endl;
//	}
	ReturnValue_t status = RETURN_OK;
	uint32_t page_size = getPageSize(packet_id.pool_index);
	if ((page_size != 0)
			&& (packet_id.packet_index < n_elements[packet_id.pool_index])) {
		uint16_t packet_position = getRawPosition(packet_id);
		uint8_t* ptr = &store[packet_id.pool_index][packet_position];
		memset(ptr, 0, page_size);
		//Set free list
		size_list[packet_id.pool_index][packet_id.packet_index] = STORAGE_FREE;
	} else {
		//pool_index or packet_index is too large
		error << "LocalPool:deleteData failed." << std::endl;
		status = ILLEGAL_STORAGE_ID;
	}
	return status;
}

template<uint8_t NUMBER_OF_POOLS>
inline void LocalPool<NUMBER_OF_POOLS>::clearStore() {
	for (uint16_t n = 0; n < NUMBER_OF_POOLS; n++) {
		memset(size_list[n], STORAGE_FREE, (n_elements[n] * sizeof(**size_list)));//TODO checkme
	}
}

template<uint8_t NUMBER_OF_POOLS>
inline ReturnValue_t LocalPool<NUMBER_OF_POOLS>::deleteData(uint8_t* ptr,
		uint32_t size, store_address_t* storeId) {
	store_address_t localId;
	ReturnValue_t result = ILLEGAL_ADDRESS;
	for (uint16_t n = 0; n < NUMBER_OF_POOLS; n++) {
		//Not sure if new allocates all stores in order. so better be careful.
		if ((store[n] <= ptr) && (&store[n][n_elements[n]*element_sizes[n]]) > ptr) {
			localId.pool_index = n;
			uint32_t deltaAddress = ptr - store[n];
			//Getting any data from the right "block" is ok. This is necessary, as IF's sometimes don't point to the first element of an object.
			localId.packet_index = deltaAddress / element_sizes[n];
			result = deleteData(localId);
//			if (deltaAddress % element_sizes[n] != 0) {
//				error << "Pool::deleteData: address not aligned!" << std::endl;
//			}
			break;
		}
	}
	if (storeId != NULL) {
		*storeId = localId;
	}
	return result;
}

template<uint8_t NUMBER_OF_POOLS>
inline ReturnValue_t LocalPool<NUMBER_OF_POOLS>::initialize() {
	ReturnValue_t result = SystemObject::initialize();
	if (result != RETURN_OK) {
		return result;
	}
	internalErrorReporter = objectManager->get<InternalErrorReporterIF>(objects::INTERNAL_ERROR_REPORTER);
	if (internalErrorReporter == NULL){
		return RETURN_FAILED;
	}

	//Check if any pool size is large than the maximum allowed.
	for (uint8_t count = 0; count < NUMBER_OF_POOLS; count++) {
		if (element_sizes[count] >= STORAGE_FREE) {
			error
					<< "LocalPool::initialize: Pool is too large! Max. allowed size is: "
					<< (STORAGE_FREE - 1) << std::endl;
			return RETURN_FAILED;
		}
	}
	return RETURN_OK;
}

#endif /* FRAMEWORK_STORAGEMANAGER_LOCALPOOL_H_ */

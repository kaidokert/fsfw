#ifndef FSFW_STORAGEMANAGER_LOCALPOOL_TPP_
#define FSFW_STORAGEMANAGER_LOCALPOOL_TPP_

#ifndef FSFW_STORAGEMANAGER_LOCALPOOL_H_
#error Include LocalPool.h before LocalPool.tpp!
#endif

template<uint8_t NUMBER_OF_POOLS>
inline LocalPool<NUMBER_OF_POOLS>::LocalPool(object_id_t setObjectId,
		const uint16_t element_sizes[NUMBER_OF_POOLS],
		const uint16_t n_elements[NUMBER_OF_POOLS], bool registered,
		bool spillsToHigherPools) :
		SystemObject(setObjectId, registered),  internalErrorReporter(nullptr),
		spillsToHigherPools(spillsToHigherPools)
{
	for (uint16_t n = 0; n < NUMBER_OF_POOLS; n++) {
		this->element_sizes[n] = element_sizes[n];
		this->n_elements[n] = n_elements[n];
		store[n] = new uint8_t[n_elements[n] * element_sizes[n]];
		size_list[n] = new uint32_t[n_elements[n]];
		memset(store[n], 0x00, (n_elements[n] * element_sizes[n]));
		//TODO checkme
		memset(size_list[n], STORAGE_FREE, (n_elements[n] * sizeof(**size_list)));
	}
}


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
		const uint8_t* data, size_t size) {
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
		size_t packet_size, uint16_t* poolIndex, uint16_t startAtIndex) {
	for (uint16_t n = startAtIndex; n < NUMBER_OF_POOLS; n++) {
		//debug << "LocalPool " << getObjectId() << "::getPoolIndex: Pool: " <<
		//		n << ", Element Size: " << element_sizes[n] << std::endl;
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
		sif::error << "LocalPool( " << std::hex << getObjectId() << std::dec
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
		// if (getObjectId() == objects::IPC_STORE && address->pool_index >= 3) {
		//	   debug << "Reserve: Pool: " << std::dec << address->pool_index <<
		//				" Index: " << address->packet_index << std::endl;
		// }

		size_list[address->pool_index][address->packet_index] = size;
	} else {
		if (!ignoreFault and internalErrorReporter != nullptr) {
			internalErrorReporter->storeFull();
		}
		// error << "LocalPool( " << std::hex << getObjectId() << std::dec
		// 			<< " )::reserveSpace: Packet store is full." << std::endl;
	}
	return status;
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
		store_address_t* storageId, const uint8_t* data, size_t size,
		bool ignoreFault) {
	ReturnValue_t status = reserveSpace(size, storageId, ignoreFault);
	if (status == RETURN_OK) {
		write(*storageId, data, size);
	}
	return status;
}

template<uint8_t NUMBER_OF_POOLS>
inline ReturnValue_t LocalPool<NUMBER_OF_POOLS>::getFreeElement(
		store_address_t* storageId, const size_t size,
		uint8_t** p_data, bool ignoreFault) {
	ReturnValue_t status = reserveSpace(size, storageId, ignoreFault);
	if (status == RETURN_OK) {
		*p_data = &store[storageId->pool_index][getRawPosition(*storageId)];
	} else {
		*p_data = NULL;
	}
	return status;
}

template<uint8_t NUMBER_OF_POOLS>
inline ConstAccessorPair LocalPool<NUMBER_OF_POOLS>::getData(
		store_address_t storeId) {
	uint8_t* tempData = nullptr;
	ConstStorageAccessor constAccessor(storeId, this);
	ReturnValue_t status = modifyData(storeId, &tempData, &constAccessor.size_);
	constAccessor.constDataPointer = tempData;
	return ConstAccessorPair(status, std::move(constAccessor));
}

template<uint8_t NUMBER_OF_POOLS>
inline ReturnValue_t LocalPool<NUMBER_OF_POOLS>::getData(store_address_t storeId,
		ConstStorageAccessor& storeAccessor) {
	uint8_t* tempData = nullptr;
	ReturnValue_t status = modifyData(storeId, &tempData, &storeAccessor.size_);
	storeAccessor.assignStore(this);
	storeAccessor.constDataPointer = tempData;
	return status;
}

template<uint8_t NUMBER_OF_POOLS>
inline ReturnValue_t LocalPool<NUMBER_OF_POOLS>::getData(
		store_address_t packet_id, const uint8_t** packet_ptr, size_t* size) {
	uint8_t* tempData = nullptr;
	ReturnValue_t status = modifyData(packet_id, &tempData, size);
	*packet_ptr = tempData;
	return status;
}

template<uint8_t NUMBER_OF_POOLS>
inline AccessorPair LocalPool<NUMBER_OF_POOLS>::modifyData(
		store_address_t storeId) {
	StorageAccessor accessor(storeId, this);
	ReturnValue_t status = modifyData(storeId, &accessor.dataPointer,
			&accessor.size_);
	accessor.assignConstPointer();
	return AccessorPair(status, std::move(accessor));
}

template<uint8_t NUMBER_OF_POOLS>
inline ReturnValue_t LocalPool<NUMBER_OF_POOLS>::modifyData(
		store_address_t storeId, StorageAccessor& storeAccessor) {
	storeAccessor.assignStore(this);
	ReturnValue_t status = modifyData(storeId, &storeAccessor.dataPointer,
			&storeAccessor.size_);
	storeAccessor.assignConstPointer();
	return status;
}

template<uint8_t NUMBER_OF_POOLS>
inline ReturnValue_t LocalPool<NUMBER_OF_POOLS>::modifyData(
		store_address_t packet_id, uint8_t** packet_ptr, size_t* size) {
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
	//if (getObjectId() == objects::IPC_STORE && packet_id.pool_index >= 3) {
	//	debug << "Delete: Pool: " << std::dec << packet_id.pool_index << " Index: "
	//	         << packet_id.packet_index << std::endl;
	//}
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
		sif::error << "LocalPool:deleteData failed." << std::endl;
		status = ILLEGAL_STORAGE_ID;
	}
	return status;
}

template<uint8_t NUMBER_OF_POOLS>
inline void LocalPool<NUMBER_OF_POOLS>::clearStore() {
	for (uint16_t n = 0; n < NUMBER_OF_POOLS; n++) {
		//TODO checkme
		memset(size_list[n], STORAGE_FREE, (n_elements[n] * sizeof(**size_list)));
	}
}

template<uint8_t NUMBER_OF_POOLS>
inline ReturnValue_t LocalPool<NUMBER_OF_POOLS>::deleteData(uint8_t* ptr,
		size_t size, store_address_t* storeId) {
	store_address_t localId;
	ReturnValue_t result = ILLEGAL_ADDRESS;
	for (uint16_t n = 0; n < NUMBER_OF_POOLS; n++) {
		//Not sure if new allocates all stores in order. so better be careful.
		if ((store[n] <= ptr) && (&store[n][n_elements[n]*element_sizes[n]]) > ptr) {
			localId.pool_index = n;
			uint32_t deltaAddress = ptr - store[n];
			// Getting any data from the right "block" is ok.
			// This is necessary, as IF's sometimes don't point to the first
			// element of an object.
			localId.packet_index = deltaAddress / element_sizes[n];
			result = deleteData(localId);
			//if (deltaAddress % element_sizes[n] != 0) {
			//	error << "Pool::deleteData: address not aligned!" << std::endl;
			//}
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
	internalErrorReporter = objectManager->get<InternalErrorReporterIF>(
			objects::INTERNAL_ERROR_REPORTER);
	if (internalErrorReporter == nullptr){
		return ObjectManagerIF::INTERNAL_ERR_REPORTER_UNINIT;
	}

	//Check if any pool size is large than the maximum allowed.
	for (uint8_t count = 0; count < NUMBER_OF_POOLS; count++) {
		if (element_sizes[count] >= STORAGE_FREE) {
			sif::error << "LocalPool::initialize: Pool is too large! "
					"Max. allowed size is: " << (STORAGE_FREE - 1) << std::endl;
			return StorageManagerIF::POOL_TOO_LARGE;
		}
	}
	return RETURN_OK;
}

#endif /* FSFW_STORAGEMANAGER_LOCALPOOL_TPP_ */

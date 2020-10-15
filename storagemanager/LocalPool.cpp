#include "LocalPool.h"
#include <FSFWConfig.h>
#include <cstring>

LocalPool::LocalPool(object_id_t setObjectId, const LocalPoolConfig& poolConfig,
		bool registered, bool spillsToHigherPools):
		SystemObject(setObjectId, registered),
		NUMBER_OF_POOLS(poolConfig.size()),
		spillsToHigherPools(spillsToHigherPools) {
	if(NUMBER_OF_POOLS == 0) {
		sif::error << "LocalPool::LocalPool: Passed pool configuration is "
				<< " invalid!" << std::endl;
	}
	max_pools_t index = 0;
	for (const auto& currentPoolConfig: poolConfig) {
		this->numberOfElements[index] = currentPoolConfig.first;
		this->elementSizes[index] = currentPoolConfig.second;
		store[index] = std::vector<uint8_t>(
				numberOfElements[index] * elementSizes[index]);
		sizeLists[index] = std::vector<size_type>(numberOfElements[index]);
		for(auto& size: sizeLists[index]) {
			size = STORAGE_FREE;
		}
		index++;
	}
}

LocalPool::~LocalPool(void) {}


ReturnValue_t LocalPool::addData(store_address_t* storageId,
        const uint8_t* data, size_t size, bool ignoreFault) {
    ReturnValue_t status = reserveSpace(size, storageId, ignoreFault);
    if (status == RETURN_OK) {
        write(*storageId, data, size);
    }
    return status;
}

ReturnValue_t LocalPool::getData(store_address_t packetId,
        const uint8_t **packetPtr, size_t *size) {
    uint8_t* tempData = nullptr;
    ReturnValue_t status = modifyData(packetId, &tempData, size);
    *packetPtr = tempData;
    return status;
}

ReturnValue_t LocalPool::getData(store_address_t storeId,
        ConstStorageAccessor& storeAccessor) {
    uint8_t* tempData = nullptr;
    ReturnValue_t status = modifyData(storeId, &tempData,
            &storeAccessor.size_);
    storeAccessor.assignStore(this);
    storeAccessor.constDataPointer = tempData;
    return status;
}

ConstAccessorPair LocalPool::getData(store_address_t storeId) {
    uint8_t* tempData = nullptr;
    ConstStorageAccessor constAccessor(storeId, this);
    ReturnValue_t status = modifyData(storeId, &tempData, &constAccessor.size_);
    constAccessor.constDataPointer = tempData;
    return ConstAccessorPair(status, std::move(constAccessor));
}

ReturnValue_t LocalPool::getFreeElement(store_address_t *storageId,
        const size_t size, uint8_t **pData, bool ignoreFault) {
    ReturnValue_t status = reserveSpace(size, storageId, ignoreFault);
    if (status == RETURN_OK) {
        *pData = &store[storageId->poolIndex][getRawPosition(*storageId)];
    }
    else {
        *pData = nullptr;
    }
    return status;
}


AccessorPair LocalPool::modifyData(store_address_t storeId) {
    StorageAccessor accessor(storeId, this);
    ReturnValue_t status = modifyData(storeId, &accessor.dataPointer,
            &accessor.size_);
    accessor.assignConstPointer();
    return AccessorPair(status, std::move(accessor));
}

ReturnValue_t LocalPool::modifyData(store_address_t storeId,
        StorageAccessor& storeAccessor) {
    storeAccessor.assignStore(this);
    ReturnValue_t status = modifyData(storeId, &storeAccessor.dataPointer,
            &storeAccessor.size_);
    storeAccessor.assignConstPointer();
    return status;
}

ReturnValue_t LocalPool::modifyData(store_address_t storeId,
        uint8_t **packetPtr, size_t *size) {
    ReturnValue_t status = RETURN_FAILED;
    if (storeId.poolIndex >= NUMBER_OF_POOLS) {
        return ILLEGAL_STORAGE_ID;
    }
    if ((storeId.packetIndex >= numberOfElements[storeId.poolIndex])) {
        return ILLEGAL_STORAGE_ID;
    }

    if (sizeLists[storeId.poolIndex][storeId.packetIndex]
            != STORAGE_FREE) {
        size_type packetPosition = getRawPosition(storeId);
        *packetPtr = &store[storeId.poolIndex][packetPosition];
        *size = sizeLists[storeId.poolIndex][storeId.packetIndex];
        status = RETURN_OK;
    }
    else {
        status = DATA_DOES_NOT_EXIST;
    }
    return status;
}

ReturnValue_t LocalPool::deleteData(store_address_t storeId) {
#if FSFW_DEBUGGING == 1
      sif::debug << "Delete: Pool: " << std::dec << storeId.poolIndex
              << " Index: " << storeId.packetIndex << std::endl;

#endif
    ReturnValue_t status = RETURN_OK;
    size_type pageSize = getPageSize(storeId.poolIndex);
    if ((pageSize != 0) and
            (storeId.packetIndex < numberOfElements[storeId.poolIndex])) {
        uint16_t packetPosition = getRawPosition(storeId);
        uint8_t* ptr = &store[storeId.poolIndex][packetPosition];
        std::memset(ptr, 0, pageSize);
        //Set free list
        sizeLists[storeId.poolIndex][storeId.packetIndex] = STORAGE_FREE;
    }
    else {
        //pool_index or packet_index is too large
        sif::error << "LocalPool::deleteData: Illegal store ID, no deletion!"
        		<< std::endl;
        status = ILLEGAL_STORAGE_ID;
    }
    return status;
}

ReturnValue_t LocalPool::deleteData(uint8_t *ptr, size_t size,
        store_address_t *storeId) {
    store_address_t localId;
    ReturnValue_t result = ILLEGAL_ADDRESS;
    for (uint16_t n = 0; n < NUMBER_OF_POOLS; n++) {
        //Not sure if new allocates all stores in order. so better be careful.
        if ((store[n].data() <= ptr) and
        		(&store[n][numberOfElements[n]*elementSizes[n]] > ptr)) {
            localId.poolIndex = n;
            uint32_t deltaAddress = ptr - store[n].data();
            // Getting any data from the right "block" is ok.
            // This is necessary, as IF's sometimes don't point to the first
            // element of an object.
            localId.packetIndex = deltaAddress / elementSizes[n];
            result = deleteData(localId);
#if FSFW_DEBUGGING == 1
            if (deltaAddress % elementSizes[n] != 0) {
                sif::error << "LocalPool::deleteData: Address not aligned!"
                        << std::endl;
            }
#endif
            break;
        }
    }
    if (storeId != nullptr) {
        *storeId = localId;
    }
    return result;
}


ReturnValue_t LocalPool::initialize() {
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
        if (elementSizes[count] >= STORAGE_FREE) {
            sif::error << "LocalPool::initialize: Pool is too large! "
                    "Max. allowed size is: " << (STORAGE_FREE - 1) << std::endl;
            return StorageManagerIF::POOL_TOO_LARGE;
        }
    }
    return HasReturnvaluesIF::RETURN_OK;
}

void LocalPool::clearStore() {
    for(auto& sizeList: sizeLists) {
        for(auto& size: sizeList) {
            size = STORAGE_FREE;
        }
//        std::memset(sizeList[index], 0xff,
//                numberOfElements[index] * sizeof(size_type));
    }

}

ReturnValue_t LocalPool::reserveSpace(const size_t size,
        store_address_t *storeId, bool ignoreFault) {
    ReturnValue_t status = getPoolIndex(size, &storeId->poolIndex);
    if (status != RETURN_OK) {
        sif::error << "LocalPool( " << std::hex << getObjectId() << std::dec
                << " )::reserveSpace: Packet too large." << std::endl;
        return status;
    }
    status = findEmpty(storeId->poolIndex, &storeId->packetIndex);
    while (status != RETURN_OK && spillsToHigherPools) {
        status = getPoolIndex(size, &storeId->poolIndex, storeId->poolIndex + 1);
        if (status != RETURN_OK) {
            //We don't find any fitting pool anymore.
            break;
        }
        status = findEmpty(storeId->poolIndex, &storeId->packetIndex);
    }
    if (status == RETURN_OK) {
#if FSFW_DEBUGGING == 1
        sif::debug << "Reserve: Pool: " << std::dec
                << storeId->poolIndex << " Index: " << storeId->packetIndex
                << std::endl;
#endif
        sizeLists[storeId->poolIndex][storeId->packetIndex] = size;
    }
    else {
        if ((not ignoreFault) and (internalErrorReporter != nullptr)) {
            internalErrorReporter->storeFull();
        }
    }
    return status;
}

void LocalPool::write(store_address_t storeId, const uint8_t *data,
        size_t size) {
    uint8_t* ptr = nullptr;
    size_type packetPosition = getRawPosition(storeId);

    // Size was checked before calling this function.
    ptr = &store[storeId.poolIndex][packetPosition];
    std::memcpy(ptr, data, size);
    sizeLists[storeId.poolIndex][storeId.packetIndex] = size;
}

LocalPool::size_type LocalPool::getPageSize(max_pools_t poolIndex) {
    if (poolIndex < NUMBER_OF_POOLS) {
        return elementSizes[poolIndex];
    }
    else {
        return 0;
    }
}

void LocalPool::setToSpillToHigherPools(bool enable) {
	this->spillsToHigherPools = enable;
}

ReturnValue_t LocalPool::getPoolIndex(size_t packetSize, uint16_t *poolIndex,
        uint16_t startAtIndex) {
    for (uint16_t n = startAtIndex; n < NUMBER_OF_POOLS; n++) {
#if FSFW_DEBUGGING == 1
        sif::debug << "LocalPool " << getObjectId() << "::getPoolIndex: Pool: "
             << n << ", Element Size: " << elementSizes[n] << std::endl;
#endif
        if (elementSizes[n] >= packetSize) {
            *poolIndex = n;
            return RETURN_OK;
        }
    }
    return DATA_TOO_LARGE;
}

LocalPool::size_type LocalPool::getRawPosition(store_address_t storeId) {
    return storeId.packetIndex * elementSizes[storeId.poolIndex];
}

ReturnValue_t LocalPool::findEmpty(n_pool_elem_t poolIndex, uint16_t *element) {
    ReturnValue_t status = DATA_STORAGE_FULL;
    for (uint16_t foundElement = 0; foundElement < numberOfElements[poolIndex];
            foundElement++) {
        if (sizeLists[poolIndex][foundElement] == STORAGE_FREE) {
            *element = foundElement;
            status = RETURN_OK;
            break;
        }
    }
    return status;
}

size_t LocalPool::getTotalSize(size_t* additionalSize) {
    size_t totalSize = 0;
    size_t sizesSize = 0;
    for(uint8_t idx = 0; idx < NUMBER_OF_POOLS; idx ++) {
        totalSize += elementSizes[idx] * numberOfElements[idx];
        sizesSize += numberOfElements[idx] * sizeof(size_type);
    }
    if(additionalSize != nullptr) {
        *additionalSize = sizesSize;
    }
    return totalSize;
}

void LocalPool::getFillCount(uint8_t *buffer, uint8_t *bytesWritten) {
	if(bytesWritten == nullptr or buffer == nullptr) {
		return;
	}

	uint16_t reservedHits = 0;
	uint8_t idx = 0;
	uint16_t sum = 0;
	for(; idx < NUMBER_OF_POOLS; idx ++) {
		for(const auto& size: sizeLists[idx]) {
			if(size != STORAGE_FREE) {
				reservedHits++;
			}
		}
		buffer[idx] = static_cast<float>(reservedHits) /
				numberOfElements[idx] * 100;
		*bytesWritten += 1;
		sum += buffer[idx];
		reservedHits = 0;
	}
	buffer[idx] = sum / NUMBER_OF_POOLS;
	*bytesWritten += 1;
}


void LocalPool::clearPage(max_pools_t  pageIndex) {
	if(pageIndex >= NUMBER_OF_POOLS) {
		return;
	}

	// Mark the storage as free
	for(auto& size: sizeLists[pageIndex]) {
		size = STORAGE_FREE;
	}

	// Set all the page content to 0.
	std::memset(store[pageIndex].data(), 0, elementSizes[pageIndex]);
}

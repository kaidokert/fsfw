#include "fsfw/storagemanager/LocalPool.h"

#include <cstring>

#include "fsfw/FSFW.h"
#include "fsfw/objectmanager/ObjectManager.h"

LocalPool::LocalPool(object_id_t setObjectId, const LocalPoolConfig& poolConfig, bool registered,
                     bool spillsToHigherPools)
    : SystemObject(setObjectId, registered),
      NUMBER_OF_SUBPOOLS(poolConfig.size()),
      spillsToHigherPools(spillsToHigherPools) {
  if (NUMBER_OF_SUBPOOLS == 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "LocalPool::LocalPool: Passed pool configuration is "
               << " invalid!" << std::endl;
#endif
  }
  max_subpools_t index = 0;
  for (const auto& currentPoolConfig : poolConfig) {
    this->numberOfElements[index] = currentPoolConfig.first;
    this->elementSizes[index] = currentPoolConfig.second;
    store[index] = std::vector<uint8_t>(numberOfElements[index] * elementSizes[index]);
    sizeLists[index] = std::vector<size_type>(numberOfElements[index]);
    for (auto& size : sizeLists[index]) {
      size = STORAGE_FREE;
    }
    index++;
  }
}

LocalPool::~LocalPool() = default;

ReturnValue_t LocalPool::addData(store_address_t* storageId, const uint8_t* data, size_t size,
                                 bool ignoreFault) {
  ReturnValue_t status = reserveSpace(size, storageId, ignoreFault);
  if (status == returnvalue::OK) {
    write(*storageId, data, size);
  }
  return status;
}

ReturnValue_t LocalPool::getData(store_address_t packetId, const uint8_t** packetPtr,
                                 size_t* size) {
  uint8_t* tempData = nullptr;
  ReturnValue_t status = modifyData(packetId, &tempData, size);
  *packetPtr = tempData;
  return status;
}

ReturnValue_t LocalPool::getFreeElement(store_address_t* storageId, const size_t size,
                                        uint8_t** pData, bool ignoreFault) {
  ReturnValue_t status = reserveSpace(size, storageId, ignoreFault);
  if (status == returnvalue::OK) {
    *pData = &store[storageId->poolIndex][getRawPosition(*storageId)];
  } else {
    *pData = nullptr;
  }
  return status;
}

ReturnValue_t LocalPool::modifyData(store_address_t storeId, uint8_t** packetPtr, size_t* size) {
  ReturnValue_t status = returnvalue::FAILED;
  if (storeId.poolIndex >= NUMBER_OF_SUBPOOLS) {
    return ILLEGAL_STORAGE_ID;
  }
  if ((storeId.packetIndex >= numberOfElements[storeId.poolIndex])) {
    return ILLEGAL_STORAGE_ID;
  }

  if (sizeLists[storeId.poolIndex][storeId.packetIndex] != STORAGE_FREE) {
    size_type packetPosition = getRawPosition(storeId);
    *packetPtr = &store[storeId.poolIndex][packetPosition];
    *size = sizeLists[storeId.poolIndex][storeId.packetIndex];
    status = returnvalue::OK;
  } else {
    status = DATA_DOES_NOT_EXIST;
  }
  return status;
}

ReturnValue_t LocalPool::deleteData(store_address_t storeId) {
#if FSFW_VERBOSE_LEVEL >= 2
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::debug << "Delete: Pool: " << std::dec << storeId.poolIndex
             << " Index: " << storeId.packetIndex << std::endl;
#endif

#endif
  ReturnValue_t status = returnvalue::OK;
  size_type pageSize = getSubpoolElementSize(storeId.poolIndex);
  if ((pageSize != 0) and (storeId.packetIndex < numberOfElements[storeId.poolIndex])) {
    uint16_t packetPosition = getRawPosition(storeId);
    uint8_t* ptr = &store[storeId.poolIndex][packetPosition];
    std::memset(ptr, 0, pageSize);
    // Set free list
    sizeLists[storeId.poolIndex][storeId.packetIndex] = STORAGE_FREE;
  } else {
    // pool_index or packet_index is too large
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "LocalPool::deleteData: Illegal store ID, no deletion!" << std::endl;
#endif
    status = ILLEGAL_STORAGE_ID;
  }
  return status;
}

ReturnValue_t LocalPool::deleteData(uint8_t* ptr, size_t size, store_address_t* storeId) {
  store_address_t localId;
  ReturnValue_t result = ILLEGAL_ADDRESS;
  for (uint16_t n = 0; n < NUMBER_OF_SUBPOOLS; n++) {
    // Not sure if new allocates all stores in order. so better be careful.
    if ((store[n].data() <= ptr) and (&store[n][numberOfElements[n] * elementSizes[n]] > ptr)) {
      localId.poolIndex = n;
      uint32_t deltaAddress = ptr - store[n].data();
      // Getting any data from the right "block" is ok.
      // This is necessary, as IF's sometimes don't point to the first
      // element of an object.
      localId.packetIndex = deltaAddress / elementSizes[n];
      result = deleteData(localId);
#if FSFW_VERBOSE_LEVEL >= 2
      if (deltaAddress % elementSizes[n] != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "LocalPool::deleteData: Address not aligned!" << std::endl;
#endif
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
  if (result != returnvalue::OK) {
    return result;
  }
  internalErrorReporter =
      ObjectManager::instance()->get<InternalErrorReporterIF>(objects::INTERNAL_ERROR_REPORTER);
  if (internalErrorReporter == nullptr) {
    return ObjectManagerIF::INTERNAL_ERR_REPORTER_UNINIT;
  }

  // Check if any pool size is large than the maximum allowed.
  for (uint8_t count = 0; count < NUMBER_OF_SUBPOOLS; count++) {
    if (elementSizes[count] >= STORAGE_FREE) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "LocalPool::initialize: Pool is too large! "
                    "Max. allowed size is: "
                 << (STORAGE_FREE - 1) << std::endl;
#endif
      return StorageManagerIF::POOL_TOO_LARGE;
    }
  }
  return returnvalue::OK;
}

void LocalPool::clearStore() {
  for (auto& sizeList : sizeLists) {
    for (auto& size : sizeList) {
      size = STORAGE_FREE;
    }
  }
}

ReturnValue_t LocalPool::reserveSpace(size_t size, store_address_t* storeId, bool ignoreFault) {
  ReturnValue_t status = getSubPoolIndex(size, &storeId->poolIndex);
  if (status != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "LocalPool( " << std::hex << getObjectId() << std::dec
               << " )::reserveSpace: Packet too large." << std::endl;
#endif
    return status;
  }
  status = findEmpty(storeId->poolIndex, &storeId->packetIndex);
  while (status != returnvalue::OK && spillsToHigherPools) {
    status = getSubPoolIndex(size, &storeId->poolIndex, storeId->poolIndex + 1);
    if (status != returnvalue::OK) {
      // We don't find any fitting pool anymore.
      break;
    }
    status = findEmpty(storeId->poolIndex, &storeId->packetIndex);
  }
  if (status == returnvalue::OK) {
#if FSFW_VERBOSE_LEVEL >= 2
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "Reserve: Pool: " << std::dec << storeId->poolIndex
               << " Index: " << storeId->packetIndex << std::endl;
#endif
#endif
    sizeLists[storeId->poolIndex][storeId->packetIndex] = size;
  } else {
    if ((not ignoreFault) and (internalErrorReporter != nullptr)) {
      internalErrorReporter->storeFull();
    }
  }
  return status;
}

void LocalPool::write(store_address_t storeId, const uint8_t* data, size_t size) {
  uint8_t* ptr = nullptr;
  size_type packetPosition = getRawPosition(storeId);

  // Size was checked before calling this function.
  ptr = &store[storeId.poolIndex][packetPosition];
  std::memcpy(ptr, data, size);
  sizeLists[storeId.poolIndex][storeId.packetIndex] = size;
}

LocalPool::size_type LocalPool::getSubpoolElementSize(max_subpools_t subpoolIndex) {
  if (subpoolIndex < NUMBER_OF_SUBPOOLS) {
    return elementSizes[subpoolIndex];
  } else {
    return 0;
  }
}

void LocalPool::setToSpillToHigherPools(bool enable) { this->spillsToHigherPools = enable; }

ReturnValue_t LocalPool::getSubPoolIndex(size_t packetSize, uint16_t* subpoolIndex,
                                         uint16_t startAtIndex) {
  for (uint16_t n = startAtIndex; n < NUMBER_OF_SUBPOOLS; n++) {
#if FSFW_VERBOSE_LEVEL >= 2
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "LocalPool " << getObjectId() << "::getPoolIndex: Pool: " << n
               << ", Element Size: " << elementSizes[n] << std::endl;
#endif
#endif
    if (elementSizes[n] >= packetSize) {
      *subpoolIndex = n;
      return returnvalue::OK;
    }
  }
  return DATA_TOO_LARGE;
}

LocalPool::size_type LocalPool::getRawPosition(store_address_t storeId) {
  return storeId.packetIndex * elementSizes[storeId.poolIndex];
}

ReturnValue_t LocalPool::findEmpty(n_pool_elem_t poolIndex, uint16_t* element) {
  ReturnValue_t status = DATA_STORAGE_FULL;
  for (uint16_t foundElement = 0; foundElement < numberOfElements[poolIndex]; foundElement++) {
    if (sizeLists[poolIndex][foundElement] == STORAGE_FREE) {
      *element = foundElement;
      status = returnvalue::OK;
      break;
    }
  }
  return status;
}

size_t LocalPool::getTotalSize(size_t* additionalSize) {
  size_t totalSize = 0;
  size_t sizesSize = 0;
  for (uint8_t idx = 0; idx < NUMBER_OF_SUBPOOLS; idx++) {
    totalSize += elementSizes[idx] * numberOfElements[idx];
    sizesSize += numberOfElements[idx] * sizeof(size_type);
  }
  if (additionalSize != nullptr) {
    *additionalSize = sizesSize;
  }
  return totalSize;
}

void LocalPool::getFillCount(uint8_t* buffer, uint8_t* bytesWritten) {
  if (bytesWritten == nullptr or buffer == nullptr) {
    return;
  }

  uint16_t reservedHits = 0;
  uint8_t idx = 0;
  uint16_t sum = 0;
  for (; idx < NUMBER_OF_SUBPOOLS; idx++) {
    for (const auto& size : sizeLists[idx]) {
      if (size != STORAGE_FREE) {
        reservedHits++;
      }
    }
    buffer[idx] = static_cast<float>(reservedHits) / numberOfElements[idx] * 100;
    *bytesWritten += 1;
    sum += buffer[idx];
    reservedHits = 0;
  }
  buffer[idx] = sum / NUMBER_OF_SUBPOOLS;
  *bytesWritten += 1;
}

void LocalPool::clearSubPool(max_subpools_t subpoolIndex) {
  if (subpoolIndex >= NUMBER_OF_SUBPOOLS) {
    return;
  }

  // Mark the storage as free
  for (auto& size : sizeLists[subpoolIndex]) {
    size = STORAGE_FREE;
  }

  // Set all the page content to 0.
  std::memset(store[subpoolIndex].data(), 0, elementSizes[subpoolIndex]);
}

LocalPool::max_subpools_t LocalPool::getNumberOfSubPools() const { return NUMBER_OF_SUBPOOLS; }

bool LocalPool::hasDataAtId(store_address_t storeId) const {
  if (storeId.poolIndex >= NUMBER_OF_SUBPOOLS) {
    return false;
  }
  if ((storeId.packetIndex >= numberOfElements[storeId.poolIndex])) {
    return false;
  }
  if (sizeLists[storeId.poolIndex][storeId.packetIndex] != STORAGE_FREE) {
    return true;
  }
  return false;
}

ReturnValue_t LocalPool::getFreeElement(store_address_t* storeId, size_t size, uint8_t** pData) {
  return StorageManagerIF::getFreeElement(storeId, size, pData);
}

ConstAccessorPair LocalPool::getData(store_address_t storeId) {
  return StorageManagerIF::getData(storeId);
}

ReturnValue_t LocalPool::addData(store_address_t* storeId, const uint8_t* data, size_t size) {
  return StorageManagerIF::addData(storeId, data, size);
}

ReturnValue_t LocalPool::getData(store_address_t storeId, ConstStorageAccessor& accessor) {
  return StorageManagerIF::getData(storeId, accessor);
}

ReturnValue_t LocalPool::modifyData(store_address_t storeId, StorageAccessor& accessor) {
  return StorageManagerIF::modifyData(storeId, accessor);
}

AccessorPair LocalPool::modifyData(store_address_t storeId) {
  return StorageManagerIF::modifyData(storeId);
}

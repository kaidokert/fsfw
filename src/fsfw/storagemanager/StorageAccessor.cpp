#include "fsfw/storagemanager/StorageAccessor.h"

#include <algorithm>

#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/storagemanager/StorageManagerIF.h"

StorageAccessor::StorageAccessor(store_address_t storeId) : ConstStorageAccessor(storeId) {}

StorageAccessor::StorageAccessor(store_address_t storeId, StorageManagerIF* store)
    : ConstStorageAccessor(storeId, store) {}

StorageAccessor& StorageAccessor::operator=(StorageAccessor&& other) noexcept {
  // Call the parent move assignment and also assign own member.
  dataPointer = other.dataPointer;
  ConstStorageAccessor::operator=(std::move(other));
  return *this;
}

// Call the parent move ctor and also transfer own member.
StorageAccessor::StorageAccessor(StorageAccessor&& other) noexcept
    : ConstStorageAccessor(std::move(other)), dataPointer(other.dataPointer) {}

ReturnValue_t StorageAccessor::getDataCopy(uint8_t* pointer, size_t maxSize) {
  if (internalState == AccessState::UNINIT) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "StorageAccessor: Not initialized!" << std::endl;
#endif
    return returnvalue::FAILED;
  }
  if (size_ > maxSize) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "StorageAccessor: Supplied buffer not large "
                  "enough"
               << std::endl;
#endif
    return returnvalue::FAILED;
  }
  std::copy(dataPointer, dataPointer + size_, pointer);
  return returnvalue::OK;
}

uint8_t* StorageAccessor::data() {
  if (internalState == AccessState::UNINIT) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "StorageAccessor: Not initialized!" << std::endl;
#endif
  }
  return dataPointer;
}

ReturnValue_t StorageAccessor::write(uint8_t* data, size_t size, uint16_t offset) {
  if (internalState == AccessState::UNINIT) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "StorageAccessor: Not initialized!" << std::endl;
#endif
    return returnvalue::FAILED;
  }
  if (offset + size > size_) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "StorageAccessor: Data too large for pool "
                  "entry!"
               << std::endl;
#endif
    return returnvalue::FAILED;
  }
  std::copy(data, data + size, dataPointer + offset);
  return returnvalue::OK;
}

void StorageAccessor::assignConstPointer() { constDataPointer = dataPointer; }

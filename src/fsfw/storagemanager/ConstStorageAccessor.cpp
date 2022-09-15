#include "fsfw/storagemanager/ConstStorageAccessor.h"

#include <algorithm>

#include "fsfw/globalfunctions/arrayprinter.h"
#include "fsfw/serviceinterface/ServiceInterfaceStream.h"
#include "fsfw/storagemanager/StorageManagerIF.h"

ConstStorageAccessor::ConstStorageAccessor(store_address_t storeId) : storeId(storeId) {}

ConstStorageAccessor::ConstStorageAccessor(store_address_t storeId, StorageManagerIF* store)
    : storeId(storeId), store(store) {
  internalState = AccessState::ASSIGNED;
}

ConstStorageAccessor::~ConstStorageAccessor() {
  if (deleteData and store != nullptr) {
    store->deleteData(storeId);
  }
}

ConstStorageAccessor::ConstStorageAccessor(ConstStorageAccessor&& other) noexcept
    : constDataPointer(other.constDataPointer),
      storeId(other.storeId),
      size_(other.size_),
      store(other.store),
      deleteData(other.deleteData),
      internalState(other.internalState) {
  // This prevent premature deletion
  other.store = nullptr;
}

ConstStorageAccessor& ConstStorageAccessor::operator=(ConstStorageAccessor&& other) noexcept {
  constDataPointer = other.constDataPointer;
  storeId = other.storeId;
  store = other.store;
  size_ = other.size_;
  deleteData = other.deleteData;
  this->store = other.store;
  // This prevents premature deletion
  other.store = nullptr;
  return *this;
}

const uint8_t* ConstStorageAccessor::data() const { return constDataPointer; }

size_t ConstStorageAccessor::size() const {
  if (internalState == AccessState::UNINIT) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "StorageAccessor: Not initialized!" << std::endl;
#endif
  }
  return size_;
}

ReturnValue_t ConstStorageAccessor::getDataCopy(uint8_t* pointer, size_t maxSize) {
  if (internalState == AccessState::UNINIT) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "StorageAccessor: Not initialized!" << std::endl;
#endif
    return returnvalue::FAILED;
  }
  if (size_ > maxSize) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "StorageAccessor: Supplied buffer not large enough" << std::endl;
#endif
    return returnvalue::FAILED;
  }
  std::copy(constDataPointer, constDataPointer + size_, pointer);
  return returnvalue::OK;
}

void ConstStorageAccessor::release() { deleteData = false; }

store_address_t ConstStorageAccessor::getId() const { return storeId; }

void ConstStorageAccessor::print() const {
  if (internalState == AccessState::UNINIT or constDataPointer == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "StorageAccessor: Not initialized!" << std::endl;
#endif
    return;
  }
  arrayprinter::print(constDataPointer, size_);
}

void ConstStorageAccessor::assignStore(StorageManagerIF* store_) {
  internalState = AccessState::ASSIGNED;
  store = store_;
}

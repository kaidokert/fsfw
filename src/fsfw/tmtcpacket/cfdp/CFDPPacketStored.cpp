#include "fsfw/tmtcpacket/cfdp/CfdpPacketStored.h"

#include "fsfw/objectmanager/ObjectManager.h"

CfdpPacketStored::CfdpPacketStored() : CfdpReader(nullptr) {}

CfdpPacketStored::CfdpPacketStored(store_address_t setAddress) : CfdpReader(nullptr) {
  this->setStoreAddress(setAddress);
}

CfdpPacketStored::CfdpPacketStored(const uint8_t* data, size_t size) : CfdpReader(data) {
  if (this->getFullSize() != size) {
    return;
  }
  if (this->checkAndSetStore()) {
<<<<<<< HEAD
    ReturnValue_t status = STORE->addData(&storeAddress, data, size);
    if (status != HasReturnvaluesIF::RETURN_OK) {
      this->setData(nullptr, -1, nullptr);
=======
    ReturnValue_t status = store->addData(&storeAddress, data, size);
    if (status != returnvalue::OK) {
      this->setData(nullptr, -1);
>>>>>>> mueller/expand-retval-if
    }
    const uint8_t* storePtr = nullptr;
    // Repoint base data pointer to the data in the store.
    STORE->getData(storeAddress, &storePtr, &size);
    this->setData(const_cast<uint8_t*>(storePtr), size, nullptr);
  }
}

ReturnValue_t CfdpPacketStored::deletePacket() {
  ReturnValue_t result = STORE->deleteData(this->storeAddress);
  this->storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
  // To circumvent size checks
  this->setData(nullptr, -1, nullptr);
  return result;
}

// CFDPPacket* CFDPPacketStored::getPacketBase() {
//     return this;
// }
void CfdpPacketStored::setStoreAddress(store_address_t setAddress) {
  this->storeAddress = setAddress;
  const uint8_t* tempData = nullptr;
  size_t tempSize;
  ReturnValue_t status = returnvalue::FAILED;
  if (this->checkAndSetStore()) {
    status = STORE->getData(this->storeAddress, &tempData, &tempSize);
  }
<<<<<<< HEAD
  if (status == StorageManagerIF::RETURN_OK) {
    this->setData(const_cast<uint8_t*>(tempData), tempSize, nullptr);
=======
  if (status == returnvalue::OK) {
    this->setData(const_cast<uint8_t*>(tempData), tempSize);
>>>>>>> mueller/expand-retval-if
  } else {
    // To circumvent size checks
    this->setData(nullptr, -1, nullptr);
    this->storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
  }
}

store_address_t CfdpPacketStored::getStoreAddress() { return this->storeAddress; }

CfdpPacketStored::~CfdpPacketStored() = default;

<<<<<<< HEAD
ReturnValue_t CfdpPacketStored::getData(const uint8_t** dataPtr, size_t* dataSize) {
  return HasReturnvaluesIF::RETURN_OK;
=======
ReturnValue_t CFDPPacketStored::getData(const uint8_t** dataPtr, size_t* dataSize) {
  return returnvalue::OK;
>>>>>>> mueller/expand-retval-if
}

// ReturnValue_t CFDPPacketStored::setData(const uint8_t *data) {
//     return returnvalue::OK;
// }

bool CfdpPacketStored::checkAndSetStore() {
  if (STORE == nullptr) {
    STORE = ObjectManager::instance()->get<StorageManagerIF>(objects::TC_STORE);
    if (STORE == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "CFDPPacketStored::CFDPPacketStored: TC Store not found!" << std::endl;
#endif
      return false;
    }
  }
  return true;
}

bool CfdpPacketStored::isSizeCorrect() {
  const uint8_t* temp_data = nullptr;
  size_t temp_size;
<<<<<<< HEAD
  ReturnValue_t status = STORE->getData(this->storeAddress, &temp_data, &temp_size);
  if (status == StorageManagerIF::RETURN_OK) {
=======
  ReturnValue_t status = store->getData(this->storeAddress, &temp_data, &temp_size);
  if (status == returnvalue::OK) {
>>>>>>> mueller/expand-retval-if
    if (this->getFullSize() == temp_size) {
      return true;
    }
  }
  return false;
}

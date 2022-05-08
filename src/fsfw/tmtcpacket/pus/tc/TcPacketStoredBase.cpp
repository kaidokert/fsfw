#include "fsfw/tmtcpacket/pus/tc/TcPacketStoredBase.h"

#include <cstring>

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/objectmanager/frameworkObjects.h"
#include "fsfw/serviceinterface.h"

StorageManagerIF* TcPacketStoredBase::STORE = nullptr;

TcPacketStoredBase::TcPacketStoredBase() {
  this->storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
  this->checkAndSetStore();
}

TcPacketStoredBase::~TcPacketStoredBase() {}

ReturnValue_t TcPacketStoredBase::getData(const uint8_t** dataPtr, size_t* dataSize) {
  auto result = TcPacketStoredBase::STORE->getData(storeAddress, dataPtr, dataSize);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    FSFW_LOGW("TcPacketStoredBase: Could not get data\n");
  }
  return result;
}

bool TcPacketStoredBase::checkAndSetStore() {
  if (TcPacketStoredBase::STORE == nullptr) {
    TcPacketStoredBase::STORE = ObjectManager::instance()->get<StorageManagerIF>(objects::TC_STORE);
    if (TcPacketStoredBase::STORE == nullptr) {
      FSFW_LOGE("TcPacketStoredBase::TcPacketStoredBase: TC Store not found\n");
      return false;
    }
  }
  return true;
}

void TcPacketStoredBase::setStoreAddress(store_address_t setAddress,
                                         RedirectableDataPointerIF* packet) {
  this->storeAddress = setAddress;
  const uint8_t* tempData = nullptr;
  size_t tempSize;
  ReturnValue_t status = StorageManagerIF::RETURN_FAILED;
  if (this->checkAndSetStore()) {
    status = TcPacketStoredBase::STORE->getData(this->storeAddress, &tempData, &tempSize);
  }

  if (status == StorageManagerIF::RETURN_OK) {
    packet->setData(const_cast<uint8_t*>(tempData), tempSize);
  } else {
    packet->setData(nullptr, -1);
    this->storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
  }
}

store_address_t TcPacketStoredBase::getStoreAddress() { return this->storeAddress; }

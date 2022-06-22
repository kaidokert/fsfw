#include "fsfw/tmtcpacket/pus/tc/TcPacketStoredBase.h"

#include <cstring>

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/objectmanager/frameworkObjects.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

StorageManagerIF* TcPacketStoredBase::STORE = nullptr;

TcPacketStoredBase::TcPacketStoredBase() {
  this->storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
  TcPacketStoredBase::checkAndSetStore();
}

TcPacketStoredBase::~TcPacketStoredBase() = default;

ReturnValue_t TcPacketStoredBase::getData(const uint8_t** dataPtr, size_t* dataSize) {
  auto result = TcPacketStoredBase::STORE->getData(storeAddress, dataPtr, dataSize);
  if (result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "TcPacketStoredBase: Could not get data" << std::endl;
#else
    sif::printWarning("TcPacketStoredBase: Could not get data!\n");
#endif
  }
  return result;
}

bool TcPacketStoredBase::checkAndSetStore() {
  if (TcPacketStoredBase::STORE == nullptr) {
    TcPacketStoredBase::STORE = ObjectManager::instance()->get<StorageManagerIF>(objects::TC_STORE);
    if (TcPacketStoredBase::STORE == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "TcPacketStoredBase::TcPacketStoredBase: TC Store not found" << std::endl;
#else
      sif::printError("TcPacketStoredBase::TcPacketStoredBase: TC Store not found\n");
#endif
      return false;
    }
  }
  return true;
}

ReturnValue_t TcPacketStoredBase::setStoreAddress(store_address_t setAddress,
                                         RedirectableDataPointerIF* packet) {
  this->storeAddress = setAddress;
  const uint8_t* tempData = nullptr;
  size_t tempSize;
  ReturnValue_t status = StorageManagerIF::RETURN_FAILED;
  if (this->checkAndSetStore()) {
    status = TcPacketStoredBase::STORE->getData(this->storeAddress, &tempData, &tempSize);
  }

  if (status == StorageManagerIF::RETURN_OK) {
    return packet->setData(const_cast<uint8_t*>(tempData), tempSize);
  } else {
    this->storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
    return packet->setData(nullptr, -1);
  }
}

store_address_t TcPacketStoredBase::getStoreAddress() { return this->storeAddress; }

#include "fsfw/tmtcpacket/pus/tc/TcPacketStoredPus.h"

#include <cstring>

#include "fsfw/serviceinterface.h"

TcPacketStoredPus::TcPacketStoredPus(uint16_t apid, uint8_t service, uint8_t subservice,
                                     uint8_t sequenceCount, const uint8_t* data, size_t size,
                                     uint8_t ack)
    : TcPacketPus(nullptr) {
  this->storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
  if (not this->checkAndSetStore()) {
    return;
  }
  uint8_t* pData = nullptr;
  ReturnValue_t returnValue = TcPacketStoredPus::STORE->getFreeElement(
      &this->storeAddress, (TC_PACKET_MIN_SIZE + size), &pData);
  if (returnValue != HasReturnvaluesIF::RETURN_OK) {
    FSFW_LOGW("TcPacketStoredBase: Could not get free element from store\n");
    return;
  }
  this->setData(pData, TC_PACKET_MIN_SIZE + size);
#if FSFW_USE_PUS_C_TELECOMMANDS == 1
  pus::PusVersion pusVersion = pus::PusVersion::PUS_C_VERSION;
#else
  pus::PusVersion pusVersion = pus::PusVersion::PUS_A_VERSION;
#endif
  initializeTcPacket(apid, sequenceCount, ack, service, subservice, pusVersion);
  std::memcpy(&tcData->appData, data, size);
  this->setPacketDataLength(size + sizeof(PUSTcDataFieldHeader) + CRC_SIZE - 1);
  this->setErrorControl();
}

TcPacketStoredPus::TcPacketStoredPus() : TcPacketStoredBase(), TcPacketPus(nullptr) {}

TcPacketStoredPus::TcPacketStoredPus(store_address_t setAddress) : TcPacketPus(nullptr) {
  TcPacketStoredBase::setStoreAddress(setAddress, this);
}

TcPacketStoredPus::TcPacketStoredPus(const uint8_t* data, size_t size) : TcPacketPus(data) {
  if (this->getFullSize() != size) {
    return;
  }
  if (this->checkAndSetStore()) {
    ReturnValue_t status = STORE->addData(&storeAddress, data, size);
    if (status != HasReturnvaluesIF::RETURN_OK) {
      this->setData(nullptr, size);
    }
    const uint8_t* storePtr = nullptr;
    // Repoint base data pointer to the data in the store.
    STORE->getData(storeAddress, &storePtr, &size);
    this->setData(const_cast<uint8_t*>(storePtr), size);
  }
}

ReturnValue_t TcPacketStoredPus::deletePacket() {
  ReturnValue_t result = this->STORE->deleteData(this->storeAddress);
  this->storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
  // To circumvent size checks
  this->setData(nullptr, -1);
  return result;
}

TcPacketPusBase* TcPacketStoredPus::getPacketBase() { return this; }

bool TcPacketStoredPus::isSizeCorrect() {
  const uint8_t* temp_data = nullptr;
  size_t temp_size;
  ReturnValue_t status = this->STORE->getData(this->storeAddress, &temp_data, &temp_size);
  if (status == StorageManagerIF::RETURN_OK) {
    if (this->getFullSize() == temp_size) {
      return true;
    }
  }
  return false;
}

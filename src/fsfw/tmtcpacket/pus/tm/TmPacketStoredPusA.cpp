#include "fsfw/tmtcpacket/pus/tm/TmPacketStoredPusA.h"

#include <cstring>

#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tmtcservices/TmTcMessage.h"

TmPacketStoredPusA::TmPacketStoredPusA(store_address_t setAddress)
    : TmPacketStoredBase(setAddress), TmPacketPusA(nullptr) {}

TmPacketStoredPusA::TmPacketStoredPusA(uint16_t apid, uint8_t service, uint8_t subservice,
                                       uint8_t packetSubcounter, const uint8_t *data, uint32_t size,
                                       const uint8_t *headerData, uint32_t headerSize)
    : TmPacketPusA(nullptr) {
  storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
  if (not TmPacketStoredBase::checkAndSetStore()) {
    return;
  }
  uint8_t *pData = nullptr;
  size_t sizeToReserve = getPacketMinimumSize() + size + headerSize;
  ReturnValue_t returnValue = store->getFreeElement(&storeAddress, sizeToReserve, &pData);

  if (returnValue != returnvalue::OK) {
    handleStoreFailure("A", returnValue, sizeToReserve);
    return;
  }
  setData(pData, sizeToReserve);
  initializeTmPacket(apid, service, subservice, packetSubcounter);
  memcpy(getSourceData(), headerData, headerSize);
  memcpy(getSourceData() + headerSize, data, size);
  setPacketDataLength(size + headerSize + sizeof(PUSTmDataFieldHeaderPusA) + CRC_SIZE - 1);
}

TmPacketStoredPusA::TmPacketStoredPusA(uint16_t apid, uint8_t service, uint8_t subservice,
                                       uint8_t packetSubcounter, SerializeIF *content,
                                       SerializeIF *header)
    : TmPacketPusA(nullptr) {
  storeAddress.raw = StorageManagerIF::INVALID_ADDRESS;
  if (not TmPacketStoredBase::checkAndSetStore()) {
    return;
  }
  size_t sourceDataSize = 0;
  if (content != nullptr) {
    sourceDataSize += content->getSerializedSize();
  }
  if (header != nullptr) {
    sourceDataSize += header->getSerializedSize();
  }
  uint8_t *pData = nullptr;
  size_t sizeToReserve = getPacketMinimumSize() + sourceDataSize;
  ReturnValue_t returnValue = store->getFreeElement(&storeAddress, sizeToReserve, &pData);
  if (returnValue != returnvalue::OK) {
    handleStoreFailure("A", returnValue, sizeToReserve);
    return;
  }
  setData(pData, sizeToReserve);
  initializeTmPacket(apid, service, subservice, packetSubcounter);
  uint8_t *putDataHere = getSourceData();
  size_t size = 0;
  if (header != nullptr) {
    header->serialize(&putDataHere, &size, sourceDataSize, SerializeIF::Endianness::BIG);
  }
  if (content != nullptr) {
    content->serialize(&putDataHere, &size, sourceDataSize, SerializeIF::Endianness::BIG);
  }
  setPacketDataLength(sourceDataSize + sizeof(PUSTmDataFieldHeaderPusA) + CRC_SIZE - 1);
}

uint8_t *TmPacketStoredPusA::getAllTmData() { return getWholeData(); }

ReturnValue_t TmPacketStoredPusA::setData(uint8_t *newPointer, size_t maxSize, void *args) {
  return TmPacketPusA::setData(newPointer, maxSize);
}

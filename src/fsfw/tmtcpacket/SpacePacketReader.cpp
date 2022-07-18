#include "fsfw/tmtcpacket/SpacePacketReader.h"

#include <cstring>

#include "fsfw/serviceinterface/ServiceInterface.h"

SpacePacketReader::SpacePacketReader(const uint8_t* setAddress) {
  this->data = reinterpret_cast<SpacePacketPointer*>(const_cast<uint8_t*>(setAddress));
}

SpacePacketReader::~SpacePacketReader() = default;

inline uint16_t SpacePacketReader::getPacketId() const {
  return ((this->data->header.packetIdHAndVersion) << 8) + this->data->header.packetIdL;
}

size_t SpacePacketReader::getFullSize() {
  // +1 is done because size in packet data length field is: size of data field -1
  return this->getPacketDataLen() + sizeof(this->data->header) + 1;
}

uint8_t* SpacePacketReader::getWholeData() { return reinterpret_cast<uint8_t*>(this->data); }

uint8_t* SpacePacketReader::getPacketData() { return &(data->packet_data); }

ReturnValue_t SpacePacketReader::setData(uint8_t* pData, size_t maxSize, void* args) {
  if (maxSize < 6) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  this->data = reinterpret_cast<SpacePacketPointer*>(const_cast<uint8_t*>(pData));
  return HasReturnvaluesIF::RETURN_OK;
}
uint16_t SpacePacketReader::getPacketSeqCtrl() const {
  return ((this->data->header.packetSeqCtrlH & 0b00111111) << 8) +
         this->data->header.packetSeqCtrlL;
}

uint16_t SpacePacketReader::getPacketDataLen() const {
  return ((this->data->header.packetLenH) << 8) + this->data->header.packetLenL;
}

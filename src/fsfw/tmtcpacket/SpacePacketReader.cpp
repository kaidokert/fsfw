#include "fsfw/tmtcpacket/SpacePacketReader.h"

#include <cstring>

#include "fsfw/serviceinterface/ServiceInterface.h"

SpacePacketReader::SpacePacketReader(const uint8_t* setAddress) {
  this->data = reinterpret_cast<SpacePacketPointer*>(const_cast<uint8_t*>(setAddress));
}

SpacePacketReader::~SpacePacketReader() = default;

ReturnValue_t SpacePacketReader::initSpacePacketHeader(bool isTelecommand, bool hasSecondaryHeader,
                                                       uint16_t apid, uint16_t sequenceCount) {
  if (data == nullptr) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "SpacePacketBase::initSpacePacketHeader: Data pointer is invalid" << std::endl;
#else
    sif::printWarning("SpacePacketBase::initSpacePacketHeader: Data pointer is invalid!\n");
#endif
#endif
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  // reset header to zero:
  memset(data, 0, sizeof(this->data->header));
  // Set TC/TM bit.
  data->header.packetIdHAndVersion = ((isTelecommand ? 1 : 0)) << 4;
  // Set secondaryHeader bit
  data->header.packetIdHAndVersion |= ((hasSecondaryHeader ? 1 : 0)) << 3;
  this->setApid(apid);
  // Always initialize as standalone packets.
  data->header.packetSeqCtrlH = 0b11000000;
  setPacketSequenceCount(sequenceCount);
  return HasReturnvaluesIF::RETURN_OK;
}

inline uint16_t SpacePacketReader::getPacketId() const {
  return ((this->data->header.packetIdHAndVersion) << 8) + this->data->header.packetIdL;
}

void SpacePacketReader::setApid(uint16_t setAPID) {
  // Use first three bits of new APID, but keep rest of packet id as it was (see specification).
  this->data->header.packetIdHAndVersion =
      (this->data->header.packetIdHAndVersion & 0b11111000) | ((setAPID & 0x0700) >> 8);
  this->data->header.packetIdL = (setAPID & 0x00FF);
}

void SpacePacketReader::setSequenceFlags(uint8_t sequenceflags) {
  this->data->header.packetSeqCtrlH &= 0x3F;
  this->data->header.packetSeqCtrlH |= sequenceflags << 6;
}

void SpacePacketReader::setPacketSequenceCount(uint16_t new_count) {
  this->data->header.packetSeqCtrlH = (this->data->header.packetSeqCtrlH & 0b11000000) |
                                      (((new_count % ccsds::LIMIT_SEQUENCE_COUNT) & 0x3F00) >> 8);
  this->data->header.packetSeqCtrlL = ((new_count % ccsds::LIMIT_SEQUENCE_COUNT) & 0x00FF);
}

void SpacePacketReader::setPacketDataLength(uint16_t new_length) {
  this->data->header.packetLenH = ((new_length & 0xFF00) >> 8);
  this->data->header.packetLenL = (new_length & 0x00FF);
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

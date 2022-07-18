#include "SpacePacketCreator.h"

#include "fsfw/serialize/SerializeAdapter.h"

SpacePacketCreator::SpacePacketCreator(uint16_t packetId_, uint16_t packetSeqCtrl_,
                                       uint16_t packetLen_, uint8_t version_) {
  packetId = packetId_;
  packetSeqCtrl = packetSeqCtrl_;
  packetLen = packetLen_;
  version = version_;
}

uint16_t SpacePacketCreator::getPacketId() const { return 0; }
uint16_t SpacePacketCreator::getPacketSeqCtrl() const { return 0; }
uint16_t SpacePacketCreator::getPacketDataLen() const { return 0; }

ReturnValue_t SpacePacketCreator::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                            SerializeIF::Endianness streamEndianness) const {
  uint16_t packetIdAndVersion = version << 13 | packetId;
  ReturnValue_t result =
      SerializeAdapter::serialize(&packetIdAndVersion, buffer, size, maxSize, streamEndianness);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  result = SerializeAdapter::serialize(&packetSeqCtrl, buffer, size, maxSize, streamEndianness);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  return SerializeAdapter::serialize(&packetLen, buffer, size, maxSize, streamEndianness);
}

size_t SpacePacketCreator::getSerializedSize() const { return 0; }
ReturnValue_t SpacePacketCreator::deSerialize(const uint8_t **buffer, size_t *size,
                                              SerializeIF::Endianness streamEndianness) {
  return HasReturnvaluesIF::RETURN_FAILED;
}

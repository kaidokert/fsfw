#include "SpacePacketCreator.h"

#include "fsfw/serialize/SerializeAdapter.h"

SpacePacketCreator::SpacePacketCreator(uint16_t packetId_, uint16_t packetSeqCtrl_,
                                       uint16_t dataLen_, uint8_t version_) {
  packetId = packetId_;
  packetSeqCtrl = packetSeqCtrl_;
  dataLen = dataLen_;
  version = version_;
  valid = true;
}

SpacePacketCreator::SpacePacketCreator(ccsds::PacketType packetType, bool secHeaderFlag,
                                       uint16_t apid, ccsds::SequenceFlags seqFlags,
                                       uint16_t seqCount, uint16_t dataLen_, uint8_t version_) {
  if (apid > ccsds::LIMIT_APID) {
    valid = false;
    return;
  }
  if (seqCount > ccsds::LIMIT_SEQUENCE_COUNT) {
    valid = false;
    return;
  }
  version = version_;
  packetId = (static_cast<uint16_t>(packetType) << 12) | (static_cast<uint16_t>(secHeaderFlag) << 11) | apid;
  packetSeqCtrl = static_cast<uint8_t>(seqFlags) << 14 | seqCount;
  dataLen = dataLen_;
}

uint16_t SpacePacketCreator::getPacketId() const { return packetId; }
uint16_t SpacePacketCreator::getPacketSeqCtrl() const { return packetSeqCtrl; }
uint16_t SpacePacketCreator::getPacketDataLen() const { return dataLen; }

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
  return SerializeAdapter::serialize(&dataLen, buffer, size, maxSize, streamEndianness);
}

size_t SpacePacketCreator::getSerializedSize() const { return 0; }
ReturnValue_t SpacePacketCreator::deSerialize(const uint8_t **buffer, size_t *size,
                                              SerializeIF::Endianness streamEndianness) {
  return HasReturnvaluesIF::RETURN_FAILED;
}
bool SpacePacketCreator::isValid() const { return valid; }

#include "SpacePacketCreator.h"

#include "fsfw/serialize/SerializeAdapter.h"

SpacePacketCreator::SpacePacketCreator(PacketId packetId_, PacketSeqCtrl psc_, uint16_t dataLen_,
                                       uint8_t version_)
    : packetId(packetId_), packetSeqCtrl(psc_) {
  dataLen = dataLen_;
  version = version_;
  checkFieldValidity();
}

SpacePacketCreator::SpacePacketCreator(ccsds::PacketType packetType, bool secHeaderFlag,
                                       uint16_t apid, ccsds::SequenceFlags seqFlags,
                                       uint16_t seqCount, uint16_t dataLen_, uint8_t version_)
    : SpacePacketCreator(PacketId(packetType, secHeaderFlag, apid),
                         PacketSeqCtrl(seqFlags, seqCount), dataLen_, version_) {
  version = version_;
  dataLen = dataLen_;
}

uint16_t SpacePacketCreator::getPacketId() const { return packetId.raw(); }
uint16_t SpacePacketCreator::getPacketSeqCtrl() const { return packetSeqCtrl.raw(); }
uint16_t SpacePacketCreator::getPacketDataLen() const { return dataLen; }

ReturnValue_t SpacePacketCreator::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                            SerializeIF::Endianness streamEndianness) const {
  if (not isValid()) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  uint16_t packetIdAndVersion = (static_cast<uint16_t>(version) << 13) | packetId.raw();
  ReturnValue_t result =
      SerializeAdapter::serialize(&packetIdAndVersion, buffer, size, maxSize, streamEndianness);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  uint16_t pscRaw = packetSeqCtrl.raw();
  result = SerializeAdapter::serialize(&pscRaw, buffer, size, maxSize, streamEndianness);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  return SerializeAdapter::serialize(&dataLen, buffer, size, maxSize, streamEndianness);
}

size_t SpacePacketCreator::getSerializedSize() const { return 6; }

ReturnValue_t SpacePacketCreator::deSerialize(const uint8_t **buffer, size_t *size,
                                              SerializeIF::Endianness streamEndianness) {
  return HasReturnvaluesIF::RETURN_FAILED;
}
bool SpacePacketCreator::isValid() const { return valid; }

ReturnValue_t SpacePacketCreator::serialize(uint8_t **buffer, size_t *size, size_t maxSize) const {
  return serialize(buffer, size, maxSize, SerializeIF::Endianness::NETWORK);
}
void SpacePacketCreator::setApid(uint16_t apid) {
  if (apid < ccsds::LIMIT_APID) {
    packetId.apid = apid;
  }
  checkFieldValidity();
}

void SpacePacketCreator::setSeqCount(uint16_t seqCount) {
  if (seqCount < ccsds::LIMIT_SEQUENCE_COUNT) {
    packetSeqCtrl.seqCount = seqCount;
  }
  checkFieldValidity();
}
void SpacePacketCreator::setSeqFlags(ccsds::SequenceFlags flags) { packetSeqCtrl.seqFlags = flags; }
void SpacePacketCreator::setDataLen(uint16_t dataLen_) { dataLen = dataLen_; }
void SpacePacketCreator::checkFieldValidity() {
  valid = true;
  if (packetId.apid > ccsds::LIMIT_APID or packetSeqCtrl.seqCount > ccsds::LIMIT_SEQUENCE_COUNT) {
    valid = false;
  }
}

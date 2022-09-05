#include "PusTcCreator.h"

#include <utility>

#include "PusTcIF.h"
#include "fsfw/globalfunctions/CRC.h"
#include "fsfw/serialize/SerializeAdapter.h"

PusTcCreator::PusTcCreator(SpacePacketParams spParams, PusTcParams pusParams)
    : spCreator(std::move(spParams)), pusParams(pusParams) {
  setup();
}

ReturnValue_t PusTcCreator::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                      SerializeIF::Endianness streamEndianness) const {
  const uint8_t *start = *buffer;
  if (*size + getSerializedSize() > maxSize) {
    return SerializeIF::BUFFER_TOO_SHORT;
  }
  if (pusParams.pusVersion != ecss::PusVersion::PUS_C) {
    return PusIF::INVALID_PUS_VERSION;
  }
  ReturnValue_t result = spCreator.serialize(buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  **buffer = pusParams.pusVersion << 4 | pusParams.ackFlags;
  *buffer += 1;
  **buffer = pusParams.service;
  *buffer += 1;
  **buffer = pusParams.subservice;
  *buffer += 1;
  *size += 3;
  result =
      SerializeAdapter::serialize(&pusParams.sourceId, buffer, size, maxSize, streamEndianness);
  if (result != returnvalue::OK) {
    return result;
  }
  if (pusParams.appData != nullptr) {
    result = pusParams.appData->serialize(buffer, size, maxSize, streamEndianness);
    if (result != returnvalue::OK) {
      return result;
    }
  }

  uint16_t crc16 = CRC::crc16ccitt(start, getFullPacketLen() - sizeof(ecss::PusChecksumT));
  return SerializeAdapter::serialize(&crc16, buffer, size, maxSize, streamEndianness);
}

void PusTcCreator::updateSpLengthField() {
  size_t len = ecss::PusTcDataFieldHeader::MIN_SIZE + sizeof(ecss::PusChecksumT);
  if (pusParams.appData != nullptr) {
    len += pusParams.appData->getSerializedSize();
  }
  spCreator.setCcsdsLenFromTotalDataFieldLen(len);
}

size_t PusTcCreator::getSerializedSize() const { return spCreator.getFullPacketLen(); }

ReturnValue_t PusTcCreator::deSerialize(const uint8_t **buffer, size_t *size,
                                        SerializeIF::Endianness streamEndianness) {
  return returnvalue::FAILED;
}

uint16_t PusTcCreator::getPacketIdRaw() const { return spCreator.getPacketIdRaw(); }

uint16_t PusTcCreator::getPacketSeqCtrlRaw() const { return spCreator.getPacketSeqCtrlRaw(); }

uint16_t PusTcCreator::getPacketDataLen() const { return spCreator.getPacketDataLen(); }

uint8_t PusTcCreator::getPusVersion() const { return pusParams.pusVersion; }

uint8_t PusTcCreator::getAcknowledgeFlags() const { return pusParams.ackFlags; }

uint8_t PusTcCreator::getService() const { return pusParams.service; }

uint8_t PusTcCreator::getSubService() const { return pusParams.subservice; }

uint16_t PusTcCreator::getSourceId() const { return pusParams.sourceId; }

PusTcParams &PusTcCreator::getPusParams() { return pusParams; }

SpacePacketParams &PusTcCreator::getSpParams() { return spCreator.getParams(); }

ReturnValue_t PusTcCreator::setRawUserData(const uint8_t *data, size_t len) {
  // TODO: Check length field?
  pusParams.bufAdapter.setConstBuffer(data, len);
  pusParams.appData = &pusParams.bufAdapter;
  updateSpLengthField();
  return returnvalue::OK;
}

ReturnValue_t PusTcCreator::setSerializableUserData(const SerializeIF &serializable) {
  // TODO: Check length field?
  pusParams.appData = &serializable;
  updateSpLengthField();
  return returnvalue::OK;
}

void PusTcCreator::setup() {
  spCreator.setPacketType(ccsds::PacketType::TC);
  spCreator.setSecHeaderFlag();
  updateSpLengthField();
}

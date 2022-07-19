#include "PusTcCreator.h"

#include "PusTcIF.h"
#include "fsfw/globalfunctions/CRC.h"
#include "fsfw/serialize/SerializeAdapter.h"

PusTcCreator::PusTcCreator(SpacePacketParams spParams, PusTcParams pusParams)
    : spCreator(spParams), pusParams(pusParams) {
  updateSpLengthField();
}

ReturnValue_t PusTcCreator::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                      SerializeIF::Endianness streamEndianness) const {
  if (*size + PusTcIF::MIN_LEN + pusParams.appDataLen > maxSize) {
    return SerializeIF::BUFFER_TOO_SHORT;
  }
  ReturnValue_t result = spCreator.serialize(buffer, size, maxSize, streamEndianness);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  if (pusParams.pusVersion != ecss::PusVersion::PUS_C) {
    // TODO: Dedicated returnvalue
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  **buffer = pusParams.pusVersion << 4 | pusParams.ackFlags;
  *buffer += 1;
  **buffer = pusParams.service;
  *buffer += 1;
  **buffer = pusParams.subservice;
  *buffer += 1;
  result =
      SerializeAdapter::serialize(&pusParams.sourceId, buffer, size, maxSize, streamEndianness);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  std::memcpy(*buffer, pusParams.appData, pusParams.appDataLen);
  *buffer += pusParams.appDataLen;
  *size += pusParams.appDataLen;
  uint16_t crc16 = CRC::crc16ccitt(*buffer, getFullPacketLen() - 2);
  return SerializeAdapter::serialize(&crc16, buffer, size, maxSize, streamEndianness);
}

void PusTcCreator::updateSpLengthField() {
  spCreator.setDataLen(ecss::PusTcDataFieldHeader::MIN_LEN + pusParams.appDataLen + 1);
}

size_t PusTcCreator::getSerializedSize() const { return spCreator.getFullPacketLen(); }

ReturnValue_t PusTcCreator::deSerialize(const uint8_t **buffer, size_t *size,
                                        SerializeIF::Endianness streamEndianness) {
  return HasReturnvaluesIF::RETURN_FAILED;
}

uint16_t PusTcCreator::getPacketId() const { return spCreator.getPacketId(); }

uint16_t PusTcCreator::getPacketSeqCtrl() const { return spCreator.getPacketSeqCtrl(); }

uint16_t PusTcCreator::getPacketDataLen() const { return spCreator.getPacketDataLen(); }

uint8_t PusTcCreator::getPusVersion() const { return pusParams.pusVersion; }

uint8_t PusTcCreator::getAcknowledgeFlags() const { return pusParams.ackFlags; }

uint8_t PusTcCreator::getService() const { return pusParams.service; }

uint8_t PusTcCreator::getSubService() const { return pusParams.subservice; }

uint16_t PusTcCreator::getSourceId() const { return pusParams.sourceId; }

const uint8_t *PusTcCreator::getUserData(size_t &appDataLen) const {
  appDataLen = getUserDataSize();
  return pusParams.appData;
}

uint16_t PusTcCreator::getUserDataSize() const { return pusParams.appDataLen; }

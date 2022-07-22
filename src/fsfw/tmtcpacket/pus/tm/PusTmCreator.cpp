#include "PusTmCreator.h"

#include <utility>

#include "fsfw/globalfunctions/CRC.h"
#include "fsfw/timemanager/TimeStamperIF.h"

PusTmCreator::PusTmCreator(SpacePacketParams initSpParams, PusTmParams initPusParams)
    : pusParams(initPusParams), spCreator(std::move(initSpParams)) {
  setup();
}

PusTmCreator::PusTmCreator() { setup(); }

uint16_t PusTmCreator::getPacketIdRaw() const { return spCreator.getPacketIdRaw(); }

uint16_t PusTmCreator::getPacketSeqCtrlRaw() const { return spCreator.getPacketSeqCtrlRaw(); }

uint16_t PusTmCreator::getPacketDataLen() const { return spCreator.getPacketDataLen(); }

uint8_t PusTmCreator::getPusVersion() const { return pusParams.secHeader.pusVersion; }

uint8_t PusTmCreator::getService() const { return pusParams.secHeader.service; }

uint8_t PusTmCreator::getSubService() const { return pusParams.secHeader.subservice; }

PusTmParams& PusTmCreator::getParams() { return pusParams; }

void PusTmCreator::setTimeStamper(TimeStamperIF* timeStamper_) {
  pusParams.secHeader.timeStamper = timeStamper_;
}

uint8_t PusTmCreator::getScTimeRefStatus() { return pusParams.secHeader.scTimeRefStatus; }

uint16_t PusTmCreator::getMessageTypeCounter() { return pusParams.secHeader.messageTypeCounter; }

uint16_t PusTmCreator::getDestId() { return pusParams.secHeader.destId; }

ReturnValue_t PusTmCreator::serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                                      SerializeIF::Endianness streamEndianness) const {
  const uint8_t* start = *buffer;
  if (*size + getSerializedSize() > maxSize) {
    return SerializeIF::BUFFER_TOO_SHORT;
  }
  ReturnValue_t result = spCreator.serialize(buffer, size, maxSize);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  size_t userDataLen = pusParams.dataWrapper.getLength();
  **buffer =
      ((pusParams.secHeader.pusVersion << 4) & 0xF0) | (pusParams.secHeader.scTimeRefStatus & 0x0F);
  *buffer += 1;
  **buffer = pusParams.secHeader.service;
  *buffer += 1;
  **buffer = pusParams.secHeader.subservice;
  *buffer += 1;
  *size += 3;
  result = SerializeAdapter::serialize(&pusParams.secHeader.messageTypeCounter, buffer, size,
                                       maxSize, streamEndianness);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  result = SerializeAdapter::serialize(&pusParams.secHeader.destId, buffer, size, maxSize,
                                       streamEndianness);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  result = pusParams.secHeader.timeStamper->serialize(buffer, size, maxSize, streamEndianness);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  if (pusParams.dataWrapper.type == ecss::DataTypes::RAW and
      pusParams.dataWrapper.dataUnion.raw.data != nullptr) {
    std::memcpy(*buffer, pusParams.dataWrapper.dataUnion.raw.data, userDataLen);
    *buffer += userDataLen;
    *size += userDataLen;
  } else if (pusParams.dataWrapper.type == ecss::DataTypes::SERIALIZABLE and
             pusParams.dataWrapper.dataUnion.serializable != nullptr) {
    result = pusParams.dataWrapper.dataUnion.serializable->serialize(buffer, size, maxSize,
                                                                     streamEndianness);
    if (result != HasReturnvaluesIF::RETURN_OK) {
      return result;
    }
  }
  uint16_t crc16 = CRC::crc16ccitt(start, getFullPacketLen() - sizeof(ecss::PusChecksumT));
  return SerializeAdapter::serialize(&crc16, buffer, size, maxSize, streamEndianness);
}

size_t PusTmCreator::getSerializedSize() const { return getFullPacketLen(); }
ReturnValue_t PusTmCreator::deSerialize(const uint8_t** buffer, size_t* size,
                                        SerializeIF::Endianness streamEndianness) {
  return HasReturnvaluesIF::RETURN_FAILED;
}

ecss::DataWrapper& PusTmCreator::getDataWrapper() { return pusParams.dataWrapper; }

TimeStamperIF* PusTmCreator::getTimestamper() const { return pusParams.secHeader.timeStamper; }

SpacePacketParams& PusTmCreator::getSpParams() { return spCreator.getParams(); }

void PusTmCreator::updateSpLengthField() {
  size_t headerLen = PusTmIF::MIN_SEC_HEADER_LEN + pusParams.dataWrapper.getLength() +
                     sizeof(ecss::PusChecksumT) - 1;
  if (pusParams.secHeader.timeStamper != nullptr) {
    headerLen += pusParams.secHeader.timeStamper->getSerializedSize();
  }
  spCreator.setDataLen(headerLen);
}

void PusTmCreator::setApid(uint16_t apid) { spCreator.setApid(apid); }

ReturnValue_t PusTmCreator::serialize(uint8_t** buffer, size_t* size, size_t maxSize) const {
  return serialize(buffer, size, maxSize, SerializeIF::Endianness::NETWORK);
}

void PusTmCreator::setup() {
  updateSpLengthField();
  spCreator.setPacketType(ccsds::PacketType::TM);
}

void PusTmCreator::setMessageTypeCounter(uint16_t messageTypeCounter) {
  pusParams.secHeader.messageTypeCounter = messageTypeCounter;
};

void PusTmCreator::setDestId(uint16_t destId) { pusParams.secHeader.destId = destId; }

void PusTmCreator::setRawSourceData(ecss::DataWrapper::BufPairT bufPair) {
  pusParams.dataWrapper.type = ecss::DataTypes::RAW;
  pusParams.dataWrapper.dataUnion.raw.data = bufPair.first;
  pusParams.dataWrapper.dataUnion.raw.len = bufPair.second;
  updateSpLengthField();
}

void PusTmCreator::setSerializableSourceData(SerializeIF* serializable) {
  pusParams.dataWrapper.type = ecss::DataTypes::SERIALIZABLE;
  pusParams.dataWrapper.dataUnion.serializable = serializable;
  updateSpLengthField();
}

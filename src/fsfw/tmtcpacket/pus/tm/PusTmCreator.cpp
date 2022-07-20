#include "PusTmCreator.h"

#include <utility>

#include "fsfw/globalfunctions/CRC.h"
#include "fsfw/timemanager/TimeStamperIF.h"

PusTmCreator::PusTmCreator(SpacePacketParams initSpParams, PusTmParams initPusParams,
                           TimeStamperIF* timeStamper)
    : pusParams(initPusParams), spCreator(std::move(initSpParams)){};

PusTmCreator::PusTmCreator(TimeStamperIF* timeStamper_) {
  pusParams.secHeader.timeStamper = timeStamper_;
};

PusTmCreator::PusTmCreator() = default;

uint16_t PusTmCreator::getPacketIdRaw() const { return 0; }
uint16_t PusTmCreator::getPacketSeqCtrlRaw() const { return 0; }
uint16_t PusTmCreator::getPacketDataLen() const { return 0; }
uint8_t PusTmCreator::getPusVersion() const { return 0; }
uint8_t PusTmCreator::getService() const { return 0; }
uint8_t PusTmCreator::getSubService() const { return 0; }
PusTmParams& PusTmCreator::getParams() { return pusParams; }
void PusTmCreator::setTimeStamper(TimeStamperIF* timeStamper_) {
  pusParams.secHeader.timeStamper = timeStamper_;
}
uint8_t PusTmCreator::getScTimeRefStatus() { return 0; }
uint16_t PusTmCreator::getMessageTypeCounter() { return 0; }
uint16_t PusTmCreator::getDestId() { return 0; }

ReturnValue_t PusTmCreator::serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                                      SerializeIF::Endianness streamEndianness) const {
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
  uint16_t crc16 = CRC::crc16ccitt(*buffer, getFullPacketLen() - 2);
  return SerializeAdapter::serialize(&crc16, buffer, size, maxSize, streamEndianness);
}

size_t PusTmCreator::getSerializedSize() const { return getFullPacketLen(); }
ReturnValue_t PusTmCreator::deSerialize(const uint8_t** buffer, size_t* size,
                                        SerializeIF::Endianness streamEndianness) {
  return HasReturnvaluesIF::RETURN_FAILED;
}

ecss::DataWrapper& PusTmCreator::getDataWrapper() { return pusParams.dataWrapper; }
TimeStamperIF* PusTmCreator::getTimestamper() { return pusParams.secHeader.timeStamper; }
SpacePacketParams& PusTmCreator::getSpParams() { return spCreator.getParams(); }

void PusTmCreator::updateSpLengthField() {
  size_t headerLen = PusTmIF::MIN_TM_SIZE;
  if (pusParams.secHeader.timeStamper != nullptr) {
    headerLen += pusParams.secHeader.timeStamper->getSerializedSize();
  }
  spCreator.setDataLen(headerLen + pusParams.dataWrapper.getLength() + 1);
}

void PusTmCreator::setApid(uint16_t apid) { spCreator.setApid(apid); };

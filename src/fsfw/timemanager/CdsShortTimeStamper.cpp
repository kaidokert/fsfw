#include "fsfw/timemanager/CdsShortTimeStamper.h"

#include <cstring>

#include "fsfw/timemanager/Clock.h"

CdsShortTimeStamper::CdsShortTimeStamper(object_id_t objectId) : SystemObject(objectId) {}

ReturnValue_t CdsShortTimeStamper::addTimeStamp(uint8_t *buffer, const uint8_t maxSize) {
  size_t serLen = 0;
  return serialize(&buffer, &serLen, maxSize, SerializeIF::Endianness::NETWORK);
}

ReturnValue_t CdsShortTimeStamper::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                             SerializeIF::Endianness streamEndianness) const {
  if (*size + getSerializedSize() > maxSize) {
    return SerializeIF::BUFFER_TOO_SHORT;
  }
  timeval now{};
  Clock::getClock_timeval(&now);
  CCSDSTime::CDS_short cds{};
  ReturnValue_t result = CCSDSTime::convertToCcsds(&cds, &now);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  std::memcpy(*buffer, &cds, sizeof(cds));
  *buffer += getSerializedSize();
  *size += getSerializedSize();
  return result;
}

size_t CdsShortTimeStamper::getSerializedSize() const { return getTimestampSize(); }

ReturnValue_t CdsShortTimeStamper::deSerialize(const uint8_t **buffer, size_t *size,
                                               SerializeIF::Endianness streamEndianness) {
  return HasReturnvaluesIF::RETURN_FAILED;
}

size_t CdsShortTimeStamper::getTimestampSize() const { return TIMESTAMP_LEN; }

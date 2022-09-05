#include "fsfw/timemanager/CdsShortTimeStamper.h"

#include <cstring>

#include "fsfw/timemanager/Clock.h"

CdsShortTimeStamper::CdsShortTimeStamper(object_id_t objectId) : SystemObject(objectId) {}

ReturnValue_t CdsShortTimeStamper::serialize(uint8_t **buffer, size_t *size, size_t maxSize,
                                             SerializeIF::Endianness streamEndianness) const {
  if (*size + getSerializedSize() > maxSize) {
    return SerializeIF::BUFFER_TOO_SHORT;
  }
  timeval now{};
  Clock::getClock_timeval(&now);
  CCSDSTime::CDS_short cds{};
  ReturnValue_t result = CCSDSTime::convertToCcsds(&cds, &now);
  if (result != returnvalue::OK) {
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
  if (size == nullptr or buffer == nullptr) {
    return returnvalue::FAILED;
  }
  if (*size < getTimestampSize()) {
    return SerializeIF::STREAM_TOO_SHORT;
  }
  size_t foundLen = 0;
  if (((**buffer >> 4) & 0b111) != CCSDSTime::TimeCodeIdentification::CDS) {
    return BAD_TIMESTAMP;
  }
  auto res = CCSDSTime::convertFromCcsds(&readTime, *buffer, &foundLen, *size);
  if (res == returnvalue::OK) {
    *size -= getSerializedSize();
    *buffer += getSerializedSize();
  }
  return res;
}

timeval &CdsShortTimeStamper::getTime() { return readTime; }

size_t CdsShortTimeStamper::getTimestampSize() const { return TIMESTAMP_LEN; }

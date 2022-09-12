#include "CcsdsTimeStampReader.h"

#include "CCSDSTime.h"

ReturnValue_t CcsdsTimestampReader::readTimeStamp(const uint8_t* buffer, uint8_t maxSize) {
  ReturnValue_t result = CCSDSTime::convertFromCcsds(&time, buffer, &timestampLen, maxSize);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  return result;
}

timeval& CcsdsTimestampReader::getTime() { return time; }
size_t CcsdsTimestampReader::getTimestampLen() { return timestampLen; }

#ifndef FSFW_TIMEMANAGER_TIMEREADERIF_H
#define FSFW_TIMEMANAGER_TIMEREADERIF_H

#include <cstdlib>

#include "fsfw/returnvalues/HasReturnvaluesIF.h"

class TimeReaderIF {
 public:
  virtual ~TimeReaderIF() = default;
  virtual ReturnValue_t readTimeStamp(const uint8_t* buffer, size_t maxSize) = 0;
  // Would be nice to have this, but the clock backend needs to be redesigned
  // virtual ReturnValue_t readTimestampLen(const uint8_t* buffer, uint8_t maxSize, size_t&
  // timestampLen) = 0;
  virtual size_t getTimestampLen() = 0;
  virtual timeval& getTime() = 0;
};

#endif  // FSFW_TIMEMANAGER_TIMEREADERIF_H

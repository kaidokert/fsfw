#ifndef FSFW_TIMEMANAGER_TIMEREADERIF_H
#define FSFW_TIMEMANAGER_TIMEREADERIF_H

#include <cstdlib>

#include "TimeStampIF.h"
#include "fsfw/returnvalues/returnvalue.h"

class TimeReaderIF : public TimeStampIF {
 public:
  ~TimeReaderIF() override = default;
  virtual ReturnValue_t readTimeStamp(const uint8_t* buffer, size_t maxSize) = 0;
  virtual timeval& getTime() = 0;
};

#endif  // FSFW_TIMEMANAGER_TIMEREADERIF_H

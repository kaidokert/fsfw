#ifndef FSFW_TIMEMANAGER_CCSDSTIMESTAMPREADER_H
#define FSFW_TIMEMANAGER_CCSDSTIMESTAMPREADER_H

#include <cstdlib>

#include "TimeReaderIF.h"

class CcsdsTimestampReader : public TimeReaderIF {
 public:
  timeval& getTime() override;

 private:
  timeval time{};
  size_t timestampLen = 0;
};

#endif  // FSFW_TIMEMANAGER_CCSDSTIMESTAMPREADER_H

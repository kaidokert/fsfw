#ifndef FSFW_TIMEMANAGER_TIMEREADERIF_H
#define FSFW_TIMEMANAGER_TIMEREADERIF_H

#include <cstdlib>

#include "TimeStampIF.h"
#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw/serialize/SerializeIF.h"

class TimeReaderIF : public SerializeIF, public TimeStampIF {
 public:
  ~TimeReaderIF() override = default;
  virtual timeval& getTime() = 0;

  [[nodiscard]] size_t getSerializedSize() const override { return getTimestampSize(); }

  ReturnValue_t readTimeStamp(const uint8_t* buf, size_t maxSize) {
    size_t dummy = 0;
    return deSerialize(buf, dummy, maxSize, SerializeIF::Endianness::NETWORK);
  }

 private:
  /**
   *   Forbidden, use dedicated IF @TimeWriterIF
   */
  [[nodiscard]] ReturnValue_t serialize(uint8_t** buffer, size_t* size, size_t maxSize,
                                        Endianness streamEndianness) const override {
    return returnvalue::FAILED;
  }
};

#endif  // FSFW_TIMEMANAGER_TIMEREADERIF_H

#ifndef FSFW_TIMEMANAGER_TIMESTAMPERIF_H_
#define FSFW_TIMEMANAGER_TIMESTAMPERIF_H_

#include "TimeStampIF.h"
#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw/serialize/SerializeIF.h"

/**
 * A class implementing this IF provides facilities to add a time stamp to the
 * buffer provided.
 * Implementors need to ensure that calling the method is thread-safe, i.e.
 * addTimeStamp may be called in parallel from a different context.
 */
class TimeWriterIF : public SerializeIF, public TimeStampIF {
 public:
  ~TimeWriterIF() override = default;
  [[nodiscard]] size_t getTimestampSize() const override { return getSerializedSize(); }

  ReturnValue_t addTimeStamp(uint8_t *buf, size_t maxSize) {
    size_t dummy = 0;
    return serialize(buf, dummy, maxSize, SerializeIF::Endianness::NETWORK);
  }

 private:
  /**
   *   Forbidden, use dedicated IF @TimeReaderIF
   */
  ReturnValue_t deSerialize(const uint8_t **buffer, size_t *size,
                            Endianness streamEndianness) override {
    return returnvalue::FAILED;
  }
};

#endif /* FSFW_TIMEMANAGER_TIMESTAMPERIF_H_ */

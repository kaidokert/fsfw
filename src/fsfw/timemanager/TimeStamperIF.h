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
class TimeStamperIF : public SerializeIF, public TimeStampIF {
 public:
  virtual ReturnValue_t addTimeStamp(uint8_t* buffer, uint8_t maxSize) = 0;
  ~TimeStamperIF() override = default;
  size_t getTimestampSize() const override { return getSerializedSize(); }

 protected:
};

#endif /* FSFW_TIMEMANAGER_TIMESTAMPERIF_H_ */

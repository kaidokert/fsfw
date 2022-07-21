#ifndef FSFW_TIMEMANAGER_TIMESTAMPERIF_H_
#define FSFW_TIMEMANAGER_TIMESTAMPERIF_H_

#include "fsfw/returnvalues/HasReturnvaluesIF.h"
#include "fsfw/serialize/SerializeIF.h"

/**
 * A class implementing this IF provides facilities to add a time stamp to the
 * buffer provided.
 * Implementors need to ensure that calling the method is thread-safe, i.e.
 * addTimeStamp may be called in parallel from a different context.
 */
class TimeStamperIF : public SerializeIF {
 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::TIME_STAMPER_IF;
  static const ReturnValue_t BAD_TIMESTAMP = MAKE_RETURN_CODE(1);

  // I am going to assume there are no larger timestamps
  static constexpr size_t MAXIMUM_TIMESTAMP_LEN = 16;

  //! This is a mission-specific constant and determines the total
  //! size reserved for timestamps.
  // static const uint8_t MISSION_TIMESTAMP_SIZE = fsfwconfig::FSFW_MISSION_TIMESTAMP_SIZE;

  [[nodiscard]] virtual size_t getTimestampSize() const = 0;
  virtual ReturnValue_t addTimeStamp(uint8_t* buffer, uint8_t maxSize) = 0;

  ~TimeStamperIF() override = default;
};

#endif /* FSFW_TIMEMANAGER_TIMESTAMPERIF_H_ */

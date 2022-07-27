#ifndef FSFW_TIMEMANAGER_TIMESTAMPIF_H
#define FSFW_TIMEMANAGER_TIMESTAMPIF_H

#include <cstddef>

#include "fsfw/retval.h"

class TimeStampIF {
 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::TIME_STAMPER_IF;
  static const ReturnValue_t BAD_TIMESTAMP = MAKE_RETURN_CODE(1);

  // I am going to assume there are no larger timestamps
  static constexpr size_t MAXIMUM_TIMESTAMP_LEN = 16;

  virtual ~TimeStampIF() = default;
  [[nodiscard]] virtual size_t getTimestampSize() const = 0;
};

#endif  // FSFW_TIMEMANAGER_TIMESTAMPIF_H

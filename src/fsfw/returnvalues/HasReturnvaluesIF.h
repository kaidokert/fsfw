#ifndef FSFW_RETURNVALUES_HASRETURNVALUESIF_H_
#define FSFW_RETURNVALUES_HASRETURNVALUESIF_H_

#include <returnvalues/classIds.h>

#include <cstdint>

#include "FwClassIds.h"

#define MAKE_RETURN_CODE(number) ((INTERFACE_ID << 8) + (number))
typedef uint16_t ReturnValue_t;

namespace result {
static constexpr ReturnValue_t OK = 0;
static constexpr ReturnValue_t FAILED = 1;

static constexpr ReturnValue_t makeCode(uint8_t classId, uint8_t number) {
  return (static_cast<ReturnValue_t>(classId) << 8) + number;
}
}  // namespace result

class HasReturnvaluesIF {
 public:
  static const ReturnValue_t RETURN_OK = result::OK;
  static const ReturnValue_t RETURN_FAILED = result::FAILED;

  virtual ~HasReturnvaluesIF() = default;

  /**
   * It is discouraged to use the input parameters 0,0 and 0,1 as this
   * will generate the RETURN_OK and RETURN_FAILED returnvalues.
   * @param interfaceId
   * @param number
   * @return
   */
  static constexpr ReturnValue_t makeReturnCode(
      uint8_t classId, uint8_t number) {
    return result::makeCode(classId, number);
  }
};

#endif /* FSFW_RETURNVALUES_HASRETURNVALUESIF_H_ */

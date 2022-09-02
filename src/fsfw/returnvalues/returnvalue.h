#ifndef FSFW_RETURNVALUES_RETURNVALUE_H_
#define FSFW_RETURNVALUES_RETURNVALUE_H_

#include <cstdint>

#include "FwClassIds.h"

#define MAKE_RETURN_CODE(number) ((INTERFACE_ID << 8) + (number))

typedef uint16_t ReturnValue_t;

namespace returnvalue {
static const ReturnValue_t OK = 0;
static const ReturnValue_t FAILED = 1;

/**
 * It is discouraged to use the input parameters 0,0 and 0,1 as this
 * will generate the returnvalue::OK and returnvalue::FAILED returnvalues.
 * @param interfaceId
 * @param number
 * @return
 */
static constexpr ReturnValue_t makeCode(uint8_t classId, uint8_t number) {
  return (static_cast<ReturnValue_t>(classId) << 8) + number;
}

}  // namespace returnvalue

#endif /* FSFW_RETURNVALUES_RETURNVALUE_H_ */

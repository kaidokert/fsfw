#include "fsfw_tests/internal/UnittDefinitions.h"

#include "fsfw/serviceinterface.h"

ReturnValue_t unitt::put_error(const std::string& errorId) {
  FSFW_LOGET("Unit Tester error: Failed at test ID {}\n", errorId);
  return HasReturnvaluesIF::RETURN_FAILED;
}

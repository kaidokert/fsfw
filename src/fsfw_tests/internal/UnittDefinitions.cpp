#include "fsfw_tests/internal/UnittDefinitions.h"

ReturnValue_t unitt::put_error(std::string errorId) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::error << "Unit Tester error: Failed at test ID " << errorId << std::endl;
#else
  sif::printError("Unit Tester error: Failed at test ID %s\n", errorId.c_str());
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
  return returnvalue::FAILED;
}

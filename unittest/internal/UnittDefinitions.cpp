#include "UnittDefinitions.h"

 ReturnValue_t unitt::put_error(std::string errorId) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
	sif::error << "Unit Tester error: Failed at test ID "
			<< errorId << std::endl;
#else
	sif::printError("Unit Tester error: Failed at test ID 0x%08x", errorId);
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
	return HasReturnvaluesIF::RETURN_FAILED;
}

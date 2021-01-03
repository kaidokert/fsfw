#include "UnittDefinitions.h"

 ReturnValue_t unitt::put_error(std::string errorId) {
#if CPP_OSTREAM_ENABLED == 1
	sif::error << "Unit Tester error: Failed at test ID "
			<< errorId << "\n" << std::flush;
#endif
	return HasReturnvaluesIF::RETURN_FAILED;
}

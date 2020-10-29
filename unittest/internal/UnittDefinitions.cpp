#include <fsfw/unittest/internal/UnittDefinitions.h>

 ReturnValue_t unitt::put_error(std::string errorId) {
	sif::error << "Unit Tester error: Failed at test ID "
			<< errorId << "\n" << std::flush;
	return HasReturnvaluesIF::RETURN_FAILED;
}

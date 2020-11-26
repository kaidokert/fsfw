#include "PollingSequenceFactory.h"

#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/devicehandlers/DeviceHandlerIF.h>
#include <fsfw/tasks/FixedTimeslotTaskIF.h>

ReturnValue_t pst::pollingSequenceInitDefault(
        FixedTimeslotTaskIF *thisSequence) {
	/* Length of a communication cycle */
	uint32_t length = thisSequence->getPeriodMs();

	/* Add polling sequence table here */

	if (thisSequence->checkSequence() == HasReturnvaluesIF::RETURN_OK) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	else {
		sif::error << "pst::pollingSequenceInitDefault: Sequence invalid!"
		        << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}


#include <PollingSequenceFactory.h>
#include <systemObjectList.h>

#include <fsfw/objectmanager/ObjectManagerIF.h>
#include <fsfw/serviceinterface/ServiceInterfaceStream.h>
#include <fsfw/devicehandlers/DeviceHandlerIF.h>
#include <fsfw/tasks/FixedTimeslotTaskIF.h>

ReturnValue_t pst::pollingSequenceInitDefault(FixedTimeslotTaskIF *thisSequence)
{
	/* Length of a communication cycle */
	uint32_t length = thisSequence->getPeriodMs();

    thisSequence->addSlot(objects::DUMMY_HANDLER,
            length * 0,  DeviceHandlerIF::SEND_WRITE);
    thisSequence->addSlot(objects::DUMMY_HANDLER,
            length * 0.25, DeviceHandlerIF::GET_WRITE);
    thisSequence->addSlot(objects::DUMMY_HANDLER,
            length * 0.5,  DeviceHandlerIF::SEND_READ);
    thisSequence->addSlot(objects::DUMMY_HANDLER,
            length * 0.75, DeviceHandlerIF::GET_READ);

	if (thisSequence->checkSequence() == HasReturnvaluesIF::RETURN_OK) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	else {
		sif::error << "PollingSequence::initialize has errors!" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}


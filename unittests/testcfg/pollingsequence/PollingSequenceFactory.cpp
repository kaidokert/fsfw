#include "PollingSequenceFactory.h"

#include <fsfw/devicehandlers/DeviceHandlerIF.h>
#include <fsfw/serviceinterface/ServiceInterface.h>
#include <fsfw/tasks/FixedTimeslotTaskIF.h>

#include "fsfw/FSFW.h"
#include "tests/TestsConfig.h"

ReturnValue_t pst::pollingSequenceInitDefault(FixedTimeslotTaskIF *thisSequence) {
  /* Length of a communication cycle */
  uint32_t length = thisSequence->getPeriodMs();

  /* Add polling sequence table here */
  thisSequence->addSlot(objects::TEST_DEVICE, 0, DeviceHandlerIF::PERFORM_OPERATION);
  thisSequence->addSlot(objects::TEST_DEVICE, 0.3, DeviceHandlerIF::SEND_WRITE);
  thisSequence->addSlot(objects::TEST_DEVICE, 0.45 * length, DeviceHandlerIF::GET_WRITE);
  thisSequence->addSlot(objects::TEST_DEVICE, 0.6 * length, DeviceHandlerIF::SEND_READ);
  thisSequence->addSlot(objects::TEST_DEVICE, 0.8 * length, DeviceHandlerIF::GET_READ);

  if (thisSequence->checkSequence() == returnvalue::OK) {
    return returnvalue::OK;
  } else {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "pst::pollingSequenceInitDefault: Sequence invalid!" << std::endl;
#else
    sif::printError("pst::pollingSequenceInitDefault: Sequence invalid!");
#endif
    return returnvalue::FAILED;
  }
}

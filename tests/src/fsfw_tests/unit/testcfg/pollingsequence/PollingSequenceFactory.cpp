#include "PollingSequenceFactory.h"

#include <fsfw/devicehandlers/DeviceHandlerIF.h>
#include <fsfw/tasks/FixedTimeslotTaskIF.h>

#include "fsfw/serviceinterface.h"
#include "tests/TestsConfig.h"

ReturnValue_t pst::pollingSequenceInitDefault(FixedTimeslotTaskIF *thisSequence) {
  /* Length of a communication cycle */
  uint32_t length = thisSequence->getPeriodMs();

  /* Add polling sequence table here */
  thisSequence->addSlot(objects::TEST_DEVICE, 0 * length, DeviceHandlerIF::PERFORM_OPERATION);
  thisSequence->addSlot(objects::TEST_DEVICE, 0.3 * length, DeviceHandlerIF::SEND_WRITE);
  thisSequence->addSlot(objects::TEST_DEVICE, 0.45 * length, DeviceHandlerIF::GET_WRITE);
  thisSequence->addSlot(objects::TEST_DEVICE, 0.6 * length, DeviceHandlerIF::SEND_READ);
  thisSequence->addSlot(objects::TEST_DEVICE, 0.8 * length, DeviceHandlerIF::GET_READ);

  if (thisSequence->checkSequence() == HasReturnvaluesIF::RETURN_OK) {
    return HasReturnvaluesIF::RETURN_OK;
  } else {
    FSFW_LOGE("pst::pollingSequenceInitDefault: Sequence invalid!\n");
    return HasReturnvaluesIF::RETURN_FAILED;
  }
}

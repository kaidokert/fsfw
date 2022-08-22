#include "fsfw/osal/rtems/RTEMSTaskBase.h"

#include "fsfw/serviceinterface/ServiceInterface.h"

const size_t PeriodicTaskIF::MINIMUM_STACK_SIZE = RTEMS_MINIMUM_STACK_SIZE;

RTEMSTaskBase::RTEMSTaskBase(rtems_task_priority set_priority, size_t stack_size,
                             const char *name) {
  rtems_name osalName = 0;
  for (uint8_t i = 0; i < 4; i++) {
    if (name[i] == 0) {
      break;
    }
    osalName += name[i] << (8 * (3 - i));
  }
  // The task is created with the operating system's system call.
  rtems_status_code status = RTEMS_UNSATISFIED;
  if (set_priority <= 99) {
    status = rtems_task_create(osalName, (0xFF - 2 * set_priority), stack_size,
                               RTEMS_PREEMPT | RTEMS_NO_TIMESLICE | RTEMS_NO_ASR,
                               RTEMS_FLOATING_POINT, &id);
  }
  ReturnValue_t result = convertReturnCode(status);
  if (result != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "TaskBase::TaskBase: createTask with name " << std::hex << osalName << std::dec
               << " failed with return code " << (uint32_t)status << std::endl;
#endif
    this->id = 0;
  }
}

RTEMSTaskBase::~RTEMSTaskBase() { rtems_task_delete(id); }

rtems_id RTEMSTaskBase::getId() const { return this->id; }

ReturnValue_t RTEMSTaskBase::sleepFor(uint32_t ms) {
  rtems_status_code status = rtems_task_wake_after(RtemsBasic::convertMsToTicks(ms));
  return convertReturnCode(status);
}

ReturnValue_t RTEMSTaskBase::convertReturnCode(rtems_status_code inValue) {
  switch (inValue) {
    case RTEMS_SUCCESSFUL:
      return returnvalue::OK;
    case RTEMS_MP_NOT_CONFIGURED:
      return returnvalue::FAILED;
    case RTEMS_INVALID_NAME:
      return returnvalue::FAILED;
    case RTEMS_TOO_MANY:
      return returnvalue::FAILED;
    case RTEMS_INVALID_ADDRESS:
      return returnvalue::FAILED;
    case RTEMS_UNSATISFIED:
      return returnvalue::FAILED;
    case RTEMS_INVALID_PRIORITY:
      return returnvalue::FAILED;
    default:
      return returnvalue::FAILED;
  }
}

ReturnValue_t RTEMSTaskBase::setAndStartPeriod(rtems_interval period, rtems_id *periodId) {
  rtems_name periodName = (('P' << 24) + ('e' << 16) + ('r' << 8) + 'd');
  rtems_status_code status = rtems_rate_monotonic_create(periodName, periodId);
  if (status == RTEMS_SUCCESSFUL) {
    status = restartPeriod(period, *periodId);
  }
  return convertReturnCode(status);
}

rtems_status_code RTEMSTaskBase::restartPeriod(rtems_interval period, rtems_id periodId) {
  // This is necessary to avoid a call with period = 0, which does not start the period.
  rtems_status_code status = rtems_rate_monotonic_period(periodId, period + 1);
  return status;
}

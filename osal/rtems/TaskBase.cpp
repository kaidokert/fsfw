#include "../../serviceinterface/ServiceInterfaceStream.h"
#include "TaskBase.h"

const size_t PeriodicTaskIF::MINIMUM_STACK_SIZE=RTEMS_MINIMUM_STACK_SIZE;

TaskBase::TaskBase(rtems_task_priority set_priority, size_t stack_size,
		const char *name) {
	rtems_name osalName = 0;
	for (uint8_t i = 0; i < 4; i++) {
		if (name[i] == 0) {
			break;
		}
		osalName += name[i] << (8 * (3 - i));
	}
	//The task is created with the operating system's system call.
	rtems_status_code status = RTEMS_UNSATISFIED;
	if (set_priority >= 0 && set_priority <= 99) {
		 status = rtems_task_create(osalName,
				(0xFF - 2 * set_priority), stack_size,
				RTEMS_PREEMPT | RTEMS_NO_TIMESLICE | RTEMS_NO_ASR,
				RTEMS_FLOATING_POINT, &id);
	}
	ReturnValue_t result = convertReturnCode(status);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		sif::error << "TaskBase::TaskBase: createTask with name " << std::hex
				<< osalName << std::dec << " failed with return code "
				<< (uint32_t) status << std::endl;
		this->id = 0;
	}
}

TaskBase::~TaskBase() {
	rtems_task_delete(id);
}

rtems_id TaskBase::getId() {
	return this->id;
}

ReturnValue_t TaskBase::sleepFor(uint32_t ms) {
	rtems_status_code status = rtems_task_wake_after(RtemsBasic::convertMsToTicks(ms));
	return convertReturnCode(status);
}


ReturnValue_t TaskBase::convertReturnCode(rtems_status_code inValue) {
	switch (inValue) {
	case RTEMS_SUCCESSFUL:
		return HasReturnvaluesIF::RETURN_OK;
	case RTEMS_MP_NOT_CONFIGURED:
		return HasReturnvaluesIF::RETURN_FAILED;
	case RTEMS_INVALID_NAME:
		return HasReturnvaluesIF::RETURN_FAILED;
	case RTEMS_TOO_MANY:
		return HasReturnvaluesIF::RETURN_FAILED;
	case RTEMS_INVALID_ADDRESS:
		return HasReturnvaluesIF::RETURN_FAILED;
	case RTEMS_UNSATISFIED:
		return HasReturnvaluesIF::RETURN_FAILED;
	case RTEMS_INVALID_PRIORITY:
		return HasReturnvaluesIF::RETURN_FAILED;
	default:
		return HasReturnvaluesIF::RETURN_FAILED;
	}

}


ReturnValue_t TaskBase::setAndStartPeriod(rtems_interval period, rtems_id *periodId) {
	rtems_name periodName = (('P' << 24) + ('e' << 16) + ('r' << 8) + 'd');
	rtems_status_code status = rtems_rate_monotonic_create(periodName, periodId);
	if (status == RTEMS_SUCCESSFUL) {
		status = restartPeriod(period,*periodId);
	}
	return convertReturnCode(status);
}

rtems_status_code TaskBase::restartPeriod(rtems_interval period, rtems_id periodId){
	//This is necessary to avoid a call with period = 0, which does not start the period.
	rtems_status_code status = rtems_rate_monotonic_period(periodId, period + 1);
	return status;
}

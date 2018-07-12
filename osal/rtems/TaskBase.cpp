#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include "TaskBase.h"

const uint64_t PeriodicTaskIF::MINIMUM_STACK_SIZE=RTEMS_MINIMUM_STACK_SIZE;

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
	ReturnValue_t result = RtemsBasic::convertReturnCode(status);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		error << "TaskBase::TaskBase: createTask with name " << std::hex
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
	return RtemsBasic::convertReturnCode(status);
}

#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include "PollingTask.h"

uint32_t PollingTask::deadlineMissedCount = 0;

PollingTask::PollingTask(const char *name, rtems_task_priority setPriority,
		size_t setStack, uint32_t setOverallPeriod,
		void (*setDeadlineMissedFunc)()) :
		TaskBase(setPriority, setStack, name), periodId(0), pst(
				setOverallPeriod) {
	// All additional attributes are applied to the object.
	this->deadlineMissedFunc = setDeadlineMissedFunc;
	rtems_name periodName = (('P' << 24) + ('e' << 16) + ('r' << 8) + 'd');
	rtems_status_code status = rtems_rate_monotonic_create(periodName,
			&periodId);
	if (status != RTEMS_SUCCESSFUL) {
		error << "PollingTask::period create failed with status " << status
				<< std::endl;
	}
}

PollingTask::~PollingTask() {
}

rtems_task PollingTask::taskEntryPoint(rtems_task_argument argument) {

	//The argument is re-interpreted as PollingTask.
	PollingTask *originalTask(reinterpret_cast<PollingTask*>(argument));
	//The task's functionality is called.
	originalTask->taskFunctionality();
	debug << "Polling task " << originalTask->getId()
			<< " returned from taskFunctionality." << std::endl;
}

void PollingTask::missedDeadlineCounter() {
	PollingTask::deadlineMissedCount++;
	if (PollingTask::deadlineMissedCount % 10 == 0) {
		error << "PST missed " << PollingTask::deadlineMissedCount
				<< " deadlines." << std::endl;
	}
}

ReturnValue_t PollingTask::startTask() {
	rtems_status_code status = rtems_task_start(id, PollingTask::taskEntryPoint,
			rtems_task_argument((void *) this));
	if (status != RTEMS_SUCCESSFUL) {
		error << "PollingTask::startTask for " << std::hex << this->getId()
				<< std::dec << " failed." << std::endl;
	}
	return RtemsBasic::convertReturnCode(status);
}

ReturnValue_t PollingTask::addSlot(object_id_t componentId, uint32_t slotTimeMs,
		int8_t executionStep) {
	pst.addSlot(componentId, slotTimeMs, executionStep, this);
	return HasReturnvaluesIF::RETURN_OK;
}

uint32_t PollingTask::getPeriodMs() const {
	return pst.getLengthMs();
}

ReturnValue_t PollingTask::checkSequence() const {
	return pst.checkSequence();
}

void PollingTask::taskFunctionality() {
	// A local iterator for the Polling Sequence Table is created to find the start time for the first entry.
	std::list<FixedSequenceSlot*>::iterator it = pst.current;

	//The start time for the first entry is read.
	rtems_interval interval = RtemsBasic::convertMsToTicks(
			(*it)->pollingTimeMs);
	//The period is set up and started with the system call.
	//The +1 is necessary to avoid a call with period = 0, which does not start the period.
	rtems_status_code status = rtems_rate_monotonic_period(periodId,
			interval + 1);
	if (status != RTEMS_SUCCESSFUL) {
		error << "PollingTask::period start failed with status " << status
				<< std::endl;
		return;
	}
	//The task's "infinite" inner loop is entered.
	while (1) {
		if (pst.slotFollowsImmediately()) {
			//Do nothing
		} else {
			//The interval for the next polling slot is selected.
			interval = this->pst.getIntervalMs();
			//The period is checked and restarted with the new interval.
			//If the deadline was missed, the deadlineMissedFunc is called.
			status = rtems_rate_monotonic_period(periodId, interval);
			if (status == RTEMS_TIMEOUT) {
				if (this->deadlineMissedFunc != NULL) {
					this->deadlineMissedFunc();
				}
			}
		}
		//The device handler for this slot is executed and the next one is chosen.
		this->pst.executeAndAdvance();
	}
}

ReturnValue_t PollingTask::sleepFor(uint32_t ms){
	return TaskBase::sleepFor(ms);
};

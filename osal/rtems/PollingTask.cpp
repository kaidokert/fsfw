#include "../../tasks/FixedSequenceSlot.h"
#include "../../objectmanager/SystemObjectIF.h"
#include "../../objectmanager/ObjectManagerIF.h"
#include "PollingTask.h"
#include "RtemsBasic.h"
#include "../../returnvalues/HasReturnvaluesIF.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"
#include <rtems/bspIo.h>
#include <rtems/rtems/ratemon.h>
#include <rtems/rtems/status.h>
#include <rtems/rtems/tasks.h>
#include <rtems/rtems/types.h>
#include <stddef.h>
#include <sys/_stdint.h>
#include <iostream>
#include <list>

uint32_t PollingTask::deadlineMissedCount = 0;

PollingTask::PollingTask(const char *name, rtems_task_priority setPriority,
		size_t setStack, uint32_t setOverallPeriod,
		void (*setDeadlineMissedFunc)()) :
		TaskBase(setPriority, setStack, name), periodId(0), pst(
				setOverallPeriod) {
	// All additional attributes are applied to the object.
	this->deadlineMissedFunc = setDeadlineMissedFunc;
}

PollingTask::~PollingTask() {
}

rtems_task PollingTask::taskEntryPoint(rtems_task_argument argument) {

	//The argument is re-interpreted as PollingTask.
	PollingTask *originalTask(reinterpret_cast<PollingTask*>(argument));
	//The task's functionality is called.
	originalTask->taskFunctionality();
	sif::debug << "Polling task " << originalTask->getId()
			<< " returned from taskFunctionality." << std::endl;
}

void PollingTask::missedDeadlineCounter() {
	PollingTask::deadlineMissedCount++;
	if (PollingTask::deadlineMissedCount % 10 == 0) {
		sif::error << "PST missed " << PollingTask::deadlineMissedCount
				<< " deadlines." << std::endl;
	}
}

ReturnValue_t PollingTask::startTask() {
	rtems_status_code status = rtems_task_start(id, PollingTask::taskEntryPoint,
			rtems_task_argument((void *) this));
	if (status != RTEMS_SUCCESSFUL) {
		sif::error << "PollingTask::startTask for " << std::hex << this->getId()
				<< std::dec << " failed." << std::endl;
	}
	switch(status){
	case RTEMS_SUCCESSFUL:
		//ask started successfully
		return HasReturnvaluesIF::RETURN_OK;
	default:
/*		RTEMS_INVALID_ADDRESS - invalid task entry point
		RTEMS_INVALID_ID - invalid task id
		RTEMS_INCORRECT_STATE - task not in the dormant state
		RTEMS_ILLEGAL_ON_REMOTE_OBJECT - cannot start remote task */
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

ReturnValue_t PollingTask::addSlot(object_id_t componentId,
		uint32_t slotTimeMs, int8_t executionStep) {
	ExecutableObjectIF* object = objectManager->get<ExecutableObjectIF>(componentId);
	if (object != nullptr) {
		pst.addSlot(componentId, slotTimeMs, executionStep, object, this);
		return HasReturnvaluesIF::RETURN_OK;
	}

	sif::error << "Component " << std::hex << componentId <<
			" not found, not adding it to pst" << std::endl;
	return HasReturnvaluesIF::RETURN_FAILED;
}

uint32_t PollingTask::getPeriodMs() const {
	return pst.getLengthMs();
}

ReturnValue_t PollingTask::checkSequence() const {
	return pst.checkSequence();
}

#include <rtems/io.h>

void PollingTask::taskFunctionality() {
	// A local iterator for the Polling Sequence Table is created to find the start time for the first entry.
	FixedSlotSequence::SlotListIter it = pst.current;

	//The start time for the first entry is read.
	rtems_interval interval = RtemsBasic::convertMsToTicks(it->pollingTimeMs);
	TaskBase::setAndStartPeriod(interval,&periodId);
	//The task's "infinite" inner loop is entered.
	while (1) {
		if (pst.slotFollowsImmediately()) {
			//Do nothing
		} else {
			//The interval for the next polling slot is selected.
			interval = RtemsBasic::convertMsToTicks(this->pst.getIntervalToNextSlotMs());
			//The period is checked and restarted with the new interval.
			//If the deadline was missed, the deadlineMissedFunc is called.
			rtems_status_code status = TaskBase::restartPeriod(interval,periodId);
			if (status == RTEMS_TIMEOUT) {
				if (this->deadlineMissedFunc != nullptr) {
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

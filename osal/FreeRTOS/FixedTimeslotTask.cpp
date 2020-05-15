#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include "FixedTimeslotTask.h"

uint32_t FixedTimeslotTask::deadlineMissedCount = 0;
const size_t PeriodicTaskIF::MINIMUM_STACK_SIZE = configMINIMAL_STACK_SIZE;

FixedTimeslotTask::FixedTimeslotTask(const char *name, TaskPriority setPriority,
		TaskStackSize setStack, TaskPeriod overallPeriod,
		void (*setDeadlineMissedFunc)()) :
		started(false), handle(NULL), pst(overallPeriod * 1000) {
	xTaskCreate(taskEntryPoint, name, setStack, this, setPriority, &handle);
	// All additional attributes are applied to the object.
	this->deadlineMissedFunc = setDeadlineMissedFunc;
}

FixedTimeslotTask::~FixedTimeslotTask() {
}

void FixedTimeslotTask::taskEntryPoint(void* argument) {

	//The argument is re-interpreted as FixedTimeslotTask. The Task object is global, so it is found from any place.
	FixedTimeslotTask *originalTask(reinterpret_cast<FixedTimeslotTask*>(argument));
	// Task should not start until explicitly requested
	// in FreeRTOS, tasks start as soon as they are created if the scheduler is running
	// but not if the scheduler is not running.
	// to be able to accommodate both cases we check a member which is set in #startTask()
	// if it is not set and we get here, the scheduler was started before #startTask() was called and we need to suspend
	// if it is set, the scheduler was not running before #startTask() was called and we can continue

	if (!originalTask->started) {
		vTaskSuspend(NULL);
	}

	originalTask->taskFunctionality();
	sif::debug << "Polling task " << originalTask->handle
			<< " returned from taskFunctionality." << std::endl;
}

void FixedTimeslotTask::missedDeadlineCounter() {
	FixedTimeslotTask::deadlineMissedCount++;
	if (FixedTimeslotTask::deadlineMissedCount % 10 == 0) {
		sif::error << "PST missed " << FixedTimeslotTask::deadlineMissedCount
				<< " deadlines." << std::endl;
	}
}

ReturnValue_t FixedTimeslotTask::startTask() {
	started = true;

	// We must not call resume if scheduler is not started yet
	if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
		vTaskResume(handle);
	}

	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t FixedTimeslotTask::addSlot(object_id_t componentId,
		uint32_t slotTimeMs, int8_t executionStep) {
	if (objectManager->get<ExecutableObjectIF>(componentId) != nullptr) {
		if(slotTimeMs == 0) {
			// FreeRTOS throws a sanity error for zero values, so we set
			// the time to one millisecond.
			slotTimeMs = 1;
		}
		pst.addSlot(componentId, slotTimeMs, executionStep, this);
		return HasReturnvaluesIF::RETURN_OK;
	}

	sif::error << "Component " << std::hex << componentId <<
			" not found, not adding it to pst" << std::endl;
	return HasReturnvaluesIF::RETURN_FAILED;
}

uint32_t FixedTimeslotTask::getPeriodMs() const {
	return pst.getLengthMs();
}

ReturnValue_t FixedTimeslotTask::checkSequence() const {
	return pst.checkSequence();
}

void FixedTimeslotTask::taskFunctionality() {
	// A local iterator for the Polling Sequence Table is created to find the start time for the first entry.
	SlotListIter slotListIter = pst.current;

	//The start time for the first entry is read.
	uint32_t intervalMs = slotListIter->pollingTimeMs;
	TickType_t interval = pdMS_TO_TICKS(intervalMs);

	TickType_t xLastWakeTime;
	/* The xLastWakeTime variable needs to be initialized with the current tick
	 count. Note that this is the only time the variable is written to explicitly.
	 After this assignment, xLastWakeTime is updated automatically internally within
	 vTaskDelayUntil(). */
	xLastWakeTime = xTaskGetTickCount();

	// wait for first entry's start time
	vTaskDelayUntil(&xLastWakeTime, interval);

	/* Enter the loop that defines the task behavior. */
	for (;;) {
		//The component for this slot is executed and the next one is chosen.
		this->pst.executeAndAdvance();
		if (pst.slotFollowsImmediately()) {
			//Do nothing
		} else {
			// we need to wait before executing the current slot
			//this gives us the time to wait:
			intervalMs = this->pst.getIntervalToPreviousSlotMs();
			interval = pdMS_TO_TICKS(intervalMs);
			vTaskDelayUntil(&xLastWakeTime, interval);
			//TODO deadline missed check
		}
	}
}

ReturnValue_t FixedTimeslotTask::sleepFor(uint32_t ms) {
	vTaskDelay(pdMS_TO_TICKS(ms));
	return HasReturnvaluesIF::RETURN_OK;
}

#include "PeriodicTask.h"

#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/tasks/ExecutableObjectIF.h>

PeriodicTask::PeriodicTask(const char *name, TaskPriority setPriority,
		TaskStackSize setStack, TaskPeriod setPeriod,
		void (*setDeadlineMissedFunc)()) :
		started(false), handle(NULL), period(setPeriod), deadlineMissedFunc(
		setDeadlineMissedFunc)
{
	BaseType_t status = xTaskCreate(taskEntryPoint, name,
			setStack, this, setPriority, &handle);
	if(status != pdPASS){
		sif::debug << "PeriodicTask Insufficient heap memory remaining. "
		        "Status: " << status << std::endl;
	}

}

PeriodicTask::~PeriodicTask(void) {
	//Do not delete objects, we were responsible for ptrs only.
}

void PeriodicTask::taskEntryPoint(void* argument) {
	// The argument is re-interpreted as PeriodicTask. The Task object is
    // global, so it is found from any place.
	PeriodicTask *originalTask(reinterpret_cast<PeriodicTask*>(argument));
	/* Task should not start until explicitly requested,
	 * but in FreeRTOS, tasks start as soon as they are created if the scheduler
	 * is running but not if the scheduler is not running.
	 * To be able to accommodate both cases we check a member which is set in
	 * #startTask(). If it is not set and we get here, the scheduler was started
	 * before #startTask() was called and we need to suspend if it is set,
	 * the scheduler was not running before #startTask() was called and we
	 * can continue */

	if (not originalTask->started) {
		vTaskSuspend(NULL);
	}

	originalTask->taskFunctionality();
	sif::debug << "Polling task " << originalTask->handle
			<< " returned from taskFunctionality." << std::endl;
}

ReturnValue_t PeriodicTask::startTask() {
	started = true;

	// We must not call resume if scheduler is not started yet
	if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
		vTaskResume(handle);
	}

	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t PeriodicTask::sleepFor(uint32_t ms) {
	vTaskDelay(pdMS_TO_TICKS(ms));
	return HasReturnvaluesIF::RETURN_OK;
}

void PeriodicTask::taskFunctionality() {
	TickType_t xLastWakeTime;
	const TickType_t xPeriod = pdMS_TO_TICKS(this->period * 1000.);
	/* The xLastWakeTime variable needs to be initialized with the current tick
	 count. Note that this is the only time the variable is written to
	 explicitly. After this assignment, xLastWakeTime is updated automatically
	 internally within vTaskDelayUntil(). */
	xLastWakeTime = xTaskGetTickCount();
	/* Enter the loop that defines the task behavior. */
	for (;;) {
		for (ObjectList::iterator it = objectList.begin();
				it != objectList.end(); ++it) {
			(*it)->performOperation();
		}

		/* If all operations are finished and the difference of the
		 * current time minus the last wake time is larger than the
		 * wait period, a deadline was missed. */
		if(xTaskGetTickCount() - xLastWakeTime >= xPeriod) {
#ifdef DEBUG
			sif::warning << "PeriodicTask: " << pcTaskGetName(NULL) <<
					" missed deadline!\n" << std::flush;
#endif
			if(deadlineMissedFunc != nullptr) {
				this->deadlineMissedFunc();
			}
		}

		vTaskDelayUntil(&xLastWakeTime, xPeriod);

	}
}

ReturnValue_t PeriodicTask::addComponent(object_id_t object) {
	ExecutableObjectIF* newObject = objectManager->get<ExecutableObjectIF>(
			object);
	if (newObject == NULL) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	objectList.push_back(newObject);
	return HasReturnvaluesIF::RETURN_OK;
}

uint32_t PeriodicTask::getPeriodMs() const {
	return period * 1000;
}

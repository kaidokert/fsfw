#include "PeriodicTask.h"

#include "../../serviceinterface/ServiceInterfaceStream.h"
#include "../../tasks/ExecutableObjectIF.h"

PeriodicTask::PeriodicTask(const char *name, TaskPriority setPriority,
		TaskStackSize setStack, TaskPeriod setPeriod,
		TaskDeadlineMissedFunction deadlineMissedFunc) :
		started(false), handle(NULL), period(setPeriod), deadlineMissedFunc(
		deadlineMissedFunc)
{
	configSTACK_DEPTH_TYPE stackSize = setStack / sizeof(configSTACK_DEPTH_TYPE);
	BaseType_t status = xTaskCreate(taskEntryPoint, name,
			stackSize, this, setPriority, &handle);
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
	
	for (auto const &object: objectList) {
		object->initializeAfterTaskCreation();
	}

	/* The xLastWakeTime variable needs to be initialized with the current tick
	 count. Note that this is the only time the variable is written to
	 explicitly. After this assignment, xLastWakeTime is updated automatically
	 internally within vTaskDelayUntil(). */
	xLastWakeTime = xTaskGetTickCount();
	/* Enter the loop that defines the task behavior. */
	for (;;) {
		for (auto const& object: objectList) {
			object->performOperation();
		}

		checkMissedDeadline(xLastWakeTime, xPeriod);

		vTaskDelayUntil(&xLastWakeTime, xPeriod);

	}
}

ReturnValue_t PeriodicTask::addComponent(object_id_t object) {
	ExecutableObjectIF* newObject = objectManager->get<ExecutableObjectIF>(
			object);
	if (newObject == nullptr) {
	    sif::error << "PeriodicTask::addComponent: Invalid object. Make sure"
	            "it implement ExecutableObjectIF" << std::endl;
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	objectList.push_back(newObject);
	newObject->setTaskIF(this);

	return HasReturnvaluesIF::RETURN_OK;
}

uint32_t PeriodicTask::getPeriodMs() const {
	return period * 1000;
}

void PeriodicTask::checkMissedDeadline(const TickType_t xLastWakeTime,
        const TickType_t interval) {
    /* Check whether deadline was missed while also taking overflows
     * into account. Drawing this on paper with a timeline helps to understand
     * it. */
    TickType_t currentTickCount = xTaskGetTickCount();
    TickType_t timeToWake = xLastWakeTime + interval;
    // Time to wake has not overflown.
    if(timeToWake > xLastWakeTime) {
        /* If the current time has overflown exclusively or the current
         * tick count is simply larger than the time to wake, a deadline was
         * missed */
        if((currentTickCount < xLastWakeTime) or (currentTickCount > timeToWake)) {
            handleMissedDeadline();
        }
    }
    /* Time to wake has overflown. A deadline was missed if the current time
     * is larger than the time to wake */
    else if((timeToWake < xLastWakeTime) and (currentTickCount > timeToWake)) {
        handleMissedDeadline();
    }
}

TaskHandle_t PeriodicTask::getTaskHandle() {
    return handle;
}

void PeriodicTask::handleMissedDeadline() {
    if(deadlineMissedFunc != nullptr) {
        this->deadlineMissedFunc();
    }
}

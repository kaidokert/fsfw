#include <framework/osal/FreeRTOS/CountingSemaphUsingTask.h>
#include <framework/osal/FreeRTOS/TaskManagement.h>

CountingSemaphoreUsingTask::CountingSemaphoreUsingTask(uint8_t count,
		uint8_t initCount):
		count(count), initCount(initCount) {
	handle = TaskManagement::getCurrentTaskHandle();
}

ReturnValue_t CountingSemaphoreUsingTask::acquire(
		uint32_t timeoutMs) {
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t CountingSemaphoreUsingTask::release() {
	return HasReturnvaluesIF::RETURN_OK;
}

uint8_t CountingSemaphoreUsingTask::getSemaphoreCounter() {
	return 0;
}

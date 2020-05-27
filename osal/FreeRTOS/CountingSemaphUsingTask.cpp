#include <framework/osal/FreeRTOS/CountingSemaphUsingTask.h>
#include <framework/osal/FreeRTOS/TaskManagement.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

CountingSemaphoreUsingTask::CountingSemaphoreUsingTask(uint8_t maxCount,
		uint8_t initCount): maxCount(maxCount) {
	if(initCount > maxCount) {
		sif::error << "CountingSemaphoreUsingTask: Max count bigger than "
				"intial cout. Setting initial count to max count." << std::endl;
		initCount = maxCount;
	}
	handle = TaskManagement::getCurrentTaskHandle();
	while(currentCount != initCount) {
		xTaskNotifyGive(handle);
		currentCount++;
	}
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

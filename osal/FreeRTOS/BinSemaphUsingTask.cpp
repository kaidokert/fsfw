#include "../../osal/FreeRTOS/BinSemaphUsingTask.h"
#include "../../osal/FreeRTOS/TaskManagement.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"

BinarySemaphoreUsingTask::BinarySemaphoreUsingTask() {
	handle = TaskManagement::getCurrentTaskHandle();
	if(handle == nullptr) {
		sif::error << "Could not retrieve task handle. Please ensure the"
				"constructor was called inside a task." << std::endl;
	}
	xTaskNotifyGive(handle);
}

BinarySemaphoreUsingTask::~BinarySemaphoreUsingTask() {
	// Clear notification value on destruction.
	xTaskNotifyAndQuery(handle, 0, eSetValueWithOverwrite, nullptr);
}

ReturnValue_t BinarySemaphoreUsingTask::acquire(TimeoutType timeoutType,
        uint32_t timeoutMs) {
    TickType_t timeout = 0;
    if(timeoutType == TimeoutType::POLLING) {
        timeout = 0;
    }
    else if(timeoutType == TimeoutType::WAITING){
        timeout = pdMS_TO_TICKS(timeoutMs);
    }
    else {
        timeout = portMAX_DELAY;
    }
	return acquireWithTickTimeout(timeoutType, timeout);
}

ReturnValue_t BinarySemaphoreUsingTask::acquireWithTickTimeout(
        TimeoutType timeoutType, TickType_t timeoutTicks) {
	BaseType_t returncode = ulTaskNotifyTake(pdTRUE, timeoutTicks);
	if (returncode == pdPASS) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	else {
		return SemaphoreIF::SEMAPHORE_TIMEOUT;
	}
}

ReturnValue_t BinarySemaphoreUsingTask::release() {
	return release(this->handle);
}

ReturnValue_t BinarySemaphoreUsingTask::release(
		TaskHandle_t taskHandle) {
	if(getSemaphoreCounter(taskHandle) == 1) {
		return SemaphoreIF::SEMAPHORE_NOT_OWNED;
	}
	BaseType_t returncode = xTaskNotifyGive(taskHandle);
	if (returncode == pdPASS) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	else {
		// This should never happen.
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

TaskHandle_t BinarySemaphoreUsingTask::getTaskHandle() {
	return handle;
}

uint8_t BinarySemaphoreUsingTask::getSemaphoreCounter() const {
	return getSemaphoreCounter(this->handle);
}

uint8_t BinarySemaphoreUsingTask::getSemaphoreCounter(
		TaskHandle_t taskHandle) {
	uint32_t notificationValue;
	xTaskNotifyAndQuery(taskHandle, 0, eNoAction, &notificationValue);
	return notificationValue;
}

// Be careful with the stack size here. This is called from an ISR!
ReturnValue_t BinarySemaphoreUsingTask::releaseFromISR(
		TaskHandle_t taskHandle, BaseType_t * higherPriorityTaskWoken) {
	if(getSemaphoreCounterFromISR(taskHandle, higherPriorityTaskWoken) == 1) {
		return SemaphoreIF::SEMAPHORE_NOT_OWNED;
	}
	vTaskNotifyGiveFromISR(taskHandle, higherPriorityTaskWoken);
	return HasReturnvaluesIF::RETURN_OK;
}

uint8_t BinarySemaphoreUsingTask::getSemaphoreCounterFromISR(
		TaskHandle_t taskHandle, BaseType_t* higherPriorityTaskWoken) {
	uint32_t notificationValue = 0;
	xTaskNotifyAndQueryFromISR(taskHandle, 0, eNoAction, &notificationValue,
			higherPriorityTaskWoken);
	return notificationValue;
}

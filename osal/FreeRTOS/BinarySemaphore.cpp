#include <framework/osal/FreeRTOS/BinarySemaphore.h>
#include <framework/osal/FreeRTOS/TaskManagement.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

#if ( configUSE_TASK_NOTIFICATIONS == 0 )

BinarySemaphore::BinarySemaphore() {
	handle = xSemaphoreCreateBinary();
	if(handle == nullptr) {
		sif::error << "Semaphore: Binary semaph creation failure" << std::endl;
	}
	xSemaphoreGive(handle);
}

BinarySemaphore::~BinarySemaphore() {
	vSemaphoreDelete(handle);
}

BinarySemaphore::BinarySemaphore(BinarySemaphore&& s) {
    handle = xSemaphoreCreateBinary();
    if(handle == nullptr) {
        sif::error << "Binary semaphore creation failure" << std::endl;
    }
    xSemaphoreGive(handle);
}

BinarySemaphore& BinarySemaphore::operator =(
        BinarySemaphore&& s) {
    if(&s != this) {
        handle = xSemaphoreCreateBinary();
        if(handle == nullptr) {
            sif::error << "Binary semaphore creation failure" << std::endl;
        }
        xSemaphoreGive(handle);
    }
    return *this;
}

ReturnValue_t BinarySemaphore::takeBinarySemaphore(uint32_t timeoutMs) {
	if(handle == nullptr) {
		return SEMAPHORE_NULLPOINTER;
	}
	TickType_t timeout = SemaphoreIF::NO_TIMEOUT;
	if(timeoutMs == SemaphoreIF::MAX_TIMEOUT) {
	    timeout = SemaphoreIF::MAX_TIMEOUT;
	}
	else if(timeoutMs > BinarySemaphore::NO_TIMEOUT){
	    timeout = pdMS_TO_TICKS(timeoutMs);
	}

	BaseType_t returncode = xSemaphoreTake(handle, timeout);
	if (returncode == pdPASS) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	else {
	    return SemaphoreIF::SEMAPHORE_TIMEOUT;
	}
}

ReturnValue_t BinarySemaphore::takeBinarySemaphoreTickTimeout(
        TickType_t timeoutTicks) {
	if(handle == nullptr) {
		return SEMAPHORE_NULLPOINTER;
	}

	BaseType_t returncode = xSemaphoreTake(handle, timeoutTicks);
	if (returncode == pdPASS) {
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return SEMAPHORE_TIMEOUT;
	}
}

ReturnValue_t BinarySemaphore::giveBinarySemaphore() {
	if (handle == nullptr) {
		return SEMAPHORE_NULLPOINTER;
	}
	BaseType_t returncode = xSemaphoreGive(handle);
	if (returncode == pdPASS) {
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return SEMAPHORE_NOT_OWNED;
	}
}

SemaphoreHandle_t BinarySemaphore::getSemaphore() {
	return handle;
}

ReturnValue_t BinarySemaphore::giveBinarySemaphore(SemaphoreHandle_t semaphore) {
	if (semaphore == nullptr) {
		return SEMAPHORE_NULLPOINTER;
	}
	BaseType_t returncode = xSemaphoreGive(semaphore);
	if (returncode == pdPASS) {
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

ReturnValue_t BinarySemaphore::acquire(uint32_t timeoutMs) {
	return takeBinarySemaphore(timeoutMs);
}

ReturnValue_t BinarySemaphore::release() {
	return giveBinarySemaphore();
}

uint8_t BinarySemaphore::getSemaphoreCounter() {
	return uxSemaphoreGetCount(handle);
}

// Be careful with the stack size here. This is called from an ISR!
ReturnValue_t BinarySemaphore::giveBinarySemaphoreFromISR(SemaphoreHandle_t semaphore,
		BaseType_t * higherPriorityTaskWoken) {
	if (semaphore == nullptr) {
		return SEMAPHORE_NULLPOINTER;
	}
	BaseType_t returncode = xSemaphoreGiveFromISR(semaphore, higherPriorityTaskWoken);
	if (returncode == pdPASS) {
		if(*higherPriorityTaskWoken == pdPASS) {
			// Request context switch because unblocking the semaphore
		    // caused a high priority task unblock.
			TaskManagement::requestContextSwitch(CallContext::isr);
		}
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return SEMAPHORE_NOT_OWNED;
	}
}


#else

BinarySemaphore::BinarySemaphore() {
	handle = TaskManagement::getCurrentTaskHandle();
}

ReturnValue_t BinarySemaphore::acquire(uint32_t timeoutMs) {
	return takeBinarySemaphore(timeoutMs);
}

ReturnValue_t BinarySemaphore::release() {
	return giveBinarySemaphore();
}

ReturnValue_t BinarySemaphore::takeBinarySemaphore(uint32_t timeoutMs) {
	TickType_t timeout = SemaphoreIF::NO_TIMEOUT;
	if(timeoutMs == SemaphoreIF::MAX_TIMEOUT) {
	    timeout = SemaphoreIF::MAX_TIMEOUT;
	}
	else if(timeoutMs > BinarySemaphore::NO_TIMEOUT){
	    timeout = pdMS_TO_TICKS(timeoutMs);
	}

	BaseType_t returncode = ulTaskNotifyTake(pdTRUE, timeout);
	if (returncode == pdPASS) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	else {
	    return SemaphoreIF::SEMAPHORE_TIMEOUT;
	}
}

ReturnValue_t BinarySemaphore::takeBinarySemaphoreTickTimeout(
        TickType_t timeoutTicks) {
	BaseType_t returncode = ulTaskNotifyTake(pdTRUE, timeoutTicks);
	if (returncode == pdPASS) {
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return SEMAPHORE_TIMEOUT;
	}
}

ReturnValue_t BinarySemaphore::giveBinarySemaphore() {
	BaseType_t returncode = xTaskNotifyGive(handle);
	if (returncode == pdPASS) {
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return SEMAPHORE_NOT_OWNED;
	}
}

TaskHandle_t BinarySemaphore::getTaskHandle() {
	return handle;
}

uint8_t BinarySemaphore::getSemaphoreCounter() {
	uint32_t notificationValue;
	xTaskNotifyAndQuery(handle, 0, eNoAction, &notificationValue);
	return notificationValue;
}

uint8_t BinarySemaphore::getSemaphoreCounterFromISR(TaskHandle_t taskHandle) {
	uint32_t notificationValue;
	BaseType_t higherPriorityTaskWoken;
	xTaskNotifyAndQueryFromISR(taskHandle, 0, eNoAction, &notificationValue,
			&higherPriorityTaskWoken);
	if(higherPriorityTaskWoken) {
		TaskManagement::requestContextSwitch(CallContext::isr);
	}
	return notificationValue;
}


ReturnValue_t BinarySemaphore::giveBinarySemaphore(TaskHandle_t taskHandle) {
	BaseType_t returncode = xTaskNotifyGive(taskHandle);
	if (returncode == pdPASS) {
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return SEMAPHORE_NOT_OWNED;
	}
}

// Be careful with the stack size here. This is called from an ISR!
ReturnValue_t BinarySemaphore::giveBinarySemaphoreFromISR(
		TaskHandle_t taskHandle, BaseType_t * higherPriorityTaskWoken) {
	vTaskNotifyGiveFromISR(taskHandle, higherPriorityTaskWoken);
	if(*higherPriorityTaskWoken == pdPASS) {
		// Request context switch because unblocking the semaphore
		// caused a high priority task unblock.
		TaskManagement::requestContextSwitch(CallContext::isr);
	}
	return HasReturnvaluesIF::RETURN_OK;
}

#endif

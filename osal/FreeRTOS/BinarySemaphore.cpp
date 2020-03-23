/**
 * @file BinarySemaphore.cpp
 *
 * @date 25.02.2020
 */
#include <framework/osal/FreeRTOS/BinarySemaphore.h>
#include <framework/osal/FreeRTOS/TaskManagement.h>

#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include "portmacro.h"
#include "task.h"

BinarySemaphore::BinarySemaphore() {
	xSemaphoreCreateBinary(handle); // @suppress("Function cannot be resolved")
	if(handle == NULL) {
		error << "Binary semaphore creation failure" << std::endl;
	}
}

BinarySemaphore::~BinarySemaphore() {
	vSemaphoreDelete(handle);
}

ReturnValue_t BinarySemaphore::takeBinarySemaphore(uint32_t timeoutMs) {
	if(handle == NULL) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	TickType_t timeout = portMAX_DELAY;
	if(timeoutMs != 0) {
		timeout = pdMS_TO_TICKS(timeoutMs);
	}

	BaseType_t returncode = xSemaphoreTake(handle, timeout);
	if (returncode == pdPASS) {
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return SEMAPHORE_NOT_FOUND;
	}
}

ReturnValue_t BinarySemaphore::takeBinarySemaphoreTickTimeout(TickType_t timeoutTicks) {
	if(handle == NULL) {
		return SEMAPHORE_NOT_FOUND;
	}

	BaseType_t returncode = xSemaphoreTake(handle, timeoutTicks);
	if (returncode == pdPASS) {
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return SEMAPHORE_TIMEOUT;
	}
}

ReturnValue_t BinarySemaphore::giveBinarySemaphore() {
	if (handle == NULL) {
		return HasReturnvaluesIF::RETURN_FAILED;
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
	if (semaphore == NULL) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	BaseType_t returncode = xSemaphoreGive(semaphore);
	if (returncode == pdPASS) {
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

void BinarySemaphore::resetSemaphore() {
	vSemaphoreDelete(handle);
	xSemaphoreCreateBinary(handle);
}

ReturnValue_t BinarySemaphore::giveBinarySemaphoreFromISR(SemaphoreHandle_t semaphore,
		BaseType_t * higherPriorityTaskWoken) {
	if (semaphore == NULL) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	BaseType_t returncode = xSemaphoreGiveFromISR(semaphore, higherPriorityTaskWoken);
	if (returncode == pdPASS) {
		if(*higherPriorityTaskWoken == pdPASS) {
			// Request context switch
			TaskManagement::requestContextSwitch(CallContext::isr);
		}
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

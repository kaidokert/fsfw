#include "Mutex.h"

#include <framework/serviceinterface/ServiceInterfaceStream.h>

const uint32_t MutexIF::NO_TIMEOUT = 0;

Mutex::Mutex() {
	handle = xSemaphoreCreateMutex();
	//TODO print error
}

Mutex::~Mutex() {
	if (handle != 0) {
		vSemaphoreDelete(handle);
	}

}

ReturnValue_t Mutex::lockMutex(uint32_t timeoutMs) {
	if (handle == 0) {
		//TODO Does not exist
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	TickType_t timeout = portMAX_DELAY;
	if (timeoutMs != NO_TIMEOUT) {
		timeout = pdMS_TO_TICKS(timeoutMs);
	}

	BaseType_t returncode = xSemaphoreTake(handle, timeout);
	if (returncode == pdPASS) {
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		//TODO could not be acquired/timeout
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

ReturnValue_t Mutex::unlockMutex() {
	if (handle == 0) {
		//TODO Does not exist
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	BaseType_t returncode = xSemaphoreGive(handle);
	if (returncode == pdPASS) {
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		//TODO is not owner
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

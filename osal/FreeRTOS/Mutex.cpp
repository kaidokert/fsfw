#include "Mutex.h"

#include <framework/serviceinterface/ServiceInterfaceStream.h>

const uint32_t MutexIF::NO_TIMEOUT = 0;

Mutex::Mutex() {
	handle = xSemaphoreCreateMutex();
	if(handle == NULL) {
		sif::error << "Mutex creation failure" << std::endl;
	}
}

Mutex::~Mutex() {
	if (handle != 0) {
		vSemaphoreDelete(handle);
	}

}

ReturnValue_t Mutex::lockMutex(uint32_t timeoutMs) {
	if (handle == 0) {
		return MutexIF::MUTEX_NOT_FOUND;
	}
	TickType_t timeout = portMAX_DELAY;
	if (timeoutMs != NO_TIMEOUT) {
		timeout = pdMS_TO_TICKS(timeoutMs);
	}

	BaseType_t returncode = xSemaphoreTake(handle, timeout);
	if (returncode == pdPASS) {
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return MutexIF::MUTEX_TIMEOUT;
	}
}

ReturnValue_t Mutex::unlockMutex() {
	if (handle == 0) {
		return MutexIF::MUTEX_NOT_FOUND;
	}
	BaseType_t returncode = xSemaphoreGive(handle);
	if (returncode == pdPASS) {
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return MutexIF::CURR_THREAD_DOES_NOT_OWN_MUTEX;
	}
}

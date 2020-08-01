#include <framework/osal/FreeRTOS/Mutex.h>

#include <framework/serviceinterface/ServiceInterfaceStream.h>

const uint32_t MutexIF::POLLING = 0;
const uint32_t MutexIF::BLOCKING = portMAX_DELAY;

Mutex::Mutex() {
	handle = xSemaphoreCreateMutex();
	if(handle == nullptr) {
		sif::error << "Mutex::Mutex(FreeRTOS): Creation failure" << std::endl;
	}
}

Mutex::~Mutex() {
	if (handle != nullptr) {
		vSemaphoreDelete(handle);
	}

}

ReturnValue_t Mutex::lockMutex(uint32_t timeoutMs) {
	if (handle == nullptr) {
		return MutexIF::MUTEX_NOT_FOUND;
	}
	TickType_t timeout = MutexIF::POLLING;
	if(timeoutMs == MutexIF::BLOCKING) {
		timeout = MutexIF::BLOCKING;
	}
	else if(timeoutMs > MutexIF::POLLING){
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
	if (handle == nullptr) {
		return MutexIF::MUTEX_NOT_FOUND;
	}
	BaseType_t returncode = xSemaphoreGive(handle);
	if (returncode == pdPASS) {
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return MutexIF::CURR_THREAD_DOES_NOT_OWN_MUTEX;
	}
}

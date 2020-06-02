#include <framework/osal/FreeRTOS/BinarySemaphore.h>
#include <framework/osal/FreeRTOS/TaskManagement.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

BinarySemaphore::BinarySemaphore() {
	handle = xSemaphoreCreateBinary();
	if(handle == nullptr) {
		sif::error << "Semaphore: Binary semaph creation failure" << std::endl;
	}
	// Initiated semaphore must be given before it can be taken.
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

ReturnValue_t BinarySemaphore::acquire(uint32_t timeoutMs) {
	TickType_t timeout = SemaphoreIF::POLLING;
	if(timeoutMs == SemaphoreIF::BLOCKING) {
	    timeout = SemaphoreIF::BLOCKING;
	}
	else if(timeoutMs > SemaphoreIF::POLLING){
	    timeout = pdMS_TO_TICKS(timeoutMs);
	}
	return acquireWithTickTimeout(timeout);
}

ReturnValue_t BinarySemaphore::acquireWithTickTimeout(TickType_t timeoutTicks) {
	if(handle == nullptr) {
		return SemaphoreIF::SEMAPHORE_INVALID;
	}

	BaseType_t returncode = xSemaphoreTake(handle, timeoutTicks);
	if (returncode == pdPASS) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	else {
		return SemaphoreIF::SEMAPHORE_TIMEOUT;
	}
}

ReturnValue_t BinarySemaphore::release() {
	return release(handle);
}

ReturnValue_t BinarySemaphore::release(SemaphoreHandle_t semaphore) {
	if (semaphore == nullptr) {
		return SemaphoreIF::SEMAPHORE_INVALID;
	}
	BaseType_t returncode = xSemaphoreGive(semaphore);
	if (returncode == pdPASS) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	else {
		return SemaphoreIF::SEMAPHORE_NOT_OWNED;
	}
}

uint8_t BinarySemaphore::getSemaphoreCounter() const {
	return uxSemaphoreGetCount(handle);
}

SemaphoreHandle_t BinarySemaphore::getSemaphore() {
	return handle;
}


// Be careful with the stack size here. This is called from an ISR!
ReturnValue_t BinarySemaphore::releaseFromISR(
		SemaphoreHandle_t semaphore, BaseType_t * higherPriorityTaskWoken) {
	if (semaphore == nullptr) {
		return SemaphoreIF::SEMAPHORE_INVALID;
	}
	BaseType_t returncode = xSemaphoreGiveFromISR(semaphore,
			higherPriorityTaskWoken);
	if (returncode == pdPASS) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	else {
		return SemaphoreIF::SEMAPHORE_NOT_OWNED;
	}
}

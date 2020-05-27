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

ReturnValue_t BinarySemaphore::acquireWithTickTimeout(
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

ReturnValue_t BinarySemaphore::release() {
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

uint8_t BinarySemaphore::getSemaphoreCounter() {
	return uxSemaphoreGetCount(handle);
}

// Be careful with the stack size here. This is called from an ISR!
ReturnValue_t BinarySemaphore::giveBinarySemaphoreFromISR(
		SemaphoreHandle_t semaphore, BaseType_t * higherPriorityTaskWoken) {
	if (semaphore == nullptr) {
		return SEMAPHORE_NULLPOINTER;
	}
	BaseType_t returncode = xSemaphoreGiveFromISR(semaphore,
			higherPriorityTaskWoken);
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

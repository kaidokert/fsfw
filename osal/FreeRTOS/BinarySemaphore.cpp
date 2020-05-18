#include <framework/osal/FreeRTOS/BinarySemaphore.h>
#include <framework/osal/FreeRTOS/TaskManagement.h>

#include <framework/serviceinterface/ServiceInterfaceStream.h>

Semaphore::Semaphore() {
	handle = xSemaphoreCreateBinary();
	if(handle == nullptr) {
		sif::error << "Semaphore: Binary semaph creation failure" << std::endl;
	}
	xSemaphoreGive(handle);
}

Semaphore::~Semaphore() {
	vSemaphoreDelete(handle);
}

// This copy ctor is important as it prevents the assignment to a ressource
// (other.handle) variable which is later deleted!
Semaphore::Semaphore(const Semaphore& other) {
    handle = xSemaphoreCreateBinary();
    if(handle == nullptr) {
        sif::error << "Binary semaphore creation failure" << std::endl;
    }
    xSemaphoreGive(handle);
}

Semaphore& Semaphore::operator =(const Semaphore& s) {
    if(this != &s) {
        handle = xSemaphoreCreateBinary();
        if(handle == nullptr) {
            sif::error << "Binary semaphore creation failure" << std::endl;
        }
        xSemaphoreGive(handle);
    }
    return *this;
}

Semaphore::Semaphore(Semaphore&& s) {
    handle = xSemaphoreCreateBinary();
    if(handle == nullptr) {
        sif::error << "Binary semaphore creation failure" << std::endl;
    }
    xSemaphoreGive(handle);
}

Semaphore& Semaphore::operator =(
        Semaphore&& s) {
    if(&s != this) {
        handle = xSemaphoreCreateBinary();
        if(handle == nullptr) {
            sif::error << "Binary semaphore creation failure" << std::endl;
        }
        xSemaphoreGive(handle);
    }
    return *this;
}

ReturnValue_t Semaphore::takeBinarySemaphore(uint32_t timeoutMs) {
	if(handle == nullptr) {
		return SEMAPHORE_NULLPOINTER;
	}
	TickType_t timeout = Semaphore::NO_BLOCK_TICKS;
	if(timeoutMs == Semaphore::BLOCK_TIMEOUT) {
	    timeout = Semaphore::BLOCK_TIMEOUT_TICKS;
	}
	else if(timeoutMs > Semaphore::NO_BLOCK_TIMEOUT){
	    timeout = pdMS_TO_TICKS(timeoutMs);
	}

	BaseType_t returncode = xSemaphoreTake(handle, timeout);
	if (returncode == pdPASS) {
		return HasReturnvaluesIF::RETURN_OK;
	}
	else {
	    return SEMAPHORE_TIMEOUT;
	}
}

ReturnValue_t Semaphore::takeBinarySemaphoreTickTimeout(
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

ReturnValue_t Semaphore::giveBinarySemaphore() {
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

SemaphoreHandle_t Semaphore::getSemaphore() {
	return handle;
}

ReturnValue_t Semaphore::giveBinarySemaphore(SemaphoreHandle_t semaphore) {
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

void Semaphore::resetSemaphore() {
	if(handle != nullptr) {
		vSemaphoreDelete(handle);
		handle = xSemaphoreCreateBinary();
		xSemaphoreGive(handle);
	}
}

ReturnValue_t Semaphore::acquire(uint32_t timeoutMs) {
	return takeBinarySemaphore(timeoutMs);
}

ReturnValue_t Semaphore::release() {
	return giveBinarySemaphore();
}

uint8_t Semaphore::getSemaphoreCounter() {
	return uxSemaphoreGetCount(handle);
}

// Be careful with the stack size here. This is called from an ISR!
ReturnValue_t Semaphore::giveBinarySemaphoreFromISR(SemaphoreHandle_t semaphore,
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


/**
 * @file BinarySemaphore.cpp
 *
 * @date 25.02.2020
 */
#include <framework/osal/FreeRTOS/BinarySemaphore.h>
#include <framework/osal/FreeRTOS/TaskManagement.h>

#include <framework/serviceinterface/ServiceInterfaceStream.h>

extern "C" {
#include "portmacro.h"
#include "task.h"
}

BinarySemaphore::BinarySemaphore() {
	xSemaphoreCreateBinary(handle);
	if(handle == nullptr) {
		error << "Binary semaphore creation failure" << std::endl;
	}
}

BinarySemaphore::~BinarySemaphore() {
	vSemaphoreDelete(handle);
}

// This copy ctor is important as it prevents the assignment to a ressource
// (other.handle) variable which is later deleted!
BinarySemaphore::BinarySemaphore(const BinarySemaphore& other) {
    xSemaphoreCreateBinary(handle);
    if(handle == nullptr) {
        error << "Binary semaphore creation failure" << std::endl;
    }
}

BinarySemaphore& BinarySemaphore::operator =(const BinarySemaphore& s) {
    if(this != &s) {
        xSemaphoreCreateBinary(handle);
        if(handle == nullptr) {
            error << "Binary semaphore creation failure" << std::endl;
        }
    }
    return *this;
}

BinarySemaphore::BinarySemaphore(BinarySemaphore&& s) {
    xSemaphoreCreateBinary(handle);
    if(handle == nullptr) {
        error << "Binary semaphore creation failure" << std::endl;
    }
}

BinarySemaphore& BinarySemaphore::operator =(
        BinarySemaphore&& s) {
    if(&s != this) {
        xSemaphoreCreateBinary(handle);
        if(handle == nullptr) {
            error << "Binary semaphore creation failure" << std::endl;
        }
    }
    return *this;
}

ReturnValue_t BinarySemaphore::takeBinarySemaphore(uint32_t timeoutMs) {
	if(handle == nullptr) {
		return SEMAPHORE_NULLPOINTER;
	}
	TickType_t timeout = BinarySemaphore::NO_BLOCK_TICKS;
	if(timeoutMs == BinarySemaphore::BLOCK_TIMEOUT) {
	    timeout = BinarySemaphore::BLOCK_TIMEOUT_TICKS;
	}
	else if(timeoutMs > BinarySemaphore::NO_BLOCK_TIMEOUT){
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

void BinarySemaphore::resetSemaphore() {
	if(handle != nullptr) {
		vSemaphoreDelete(handle);
		xSemaphoreCreateBinary(handle);
	}
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

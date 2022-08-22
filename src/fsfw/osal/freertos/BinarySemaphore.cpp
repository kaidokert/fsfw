#include "fsfw/osal/freertos/BinarySemaphore.h"

#include "fsfw/osal/freertos/TaskManagement.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

BinarySemaphore::BinarySemaphore() {
  handle = xSemaphoreCreateBinary();
  if (handle == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "Semaphore: Binary semaph creation failure" << std::endl;
#endif
  }
  // Initiated semaphore must be given before it can be taken.
  xSemaphoreGive(handle);
}

BinarySemaphore::~BinarySemaphore() { vSemaphoreDelete(handle); }

BinarySemaphore::BinarySemaphore(BinarySemaphore&& s) {
  handle = xSemaphoreCreateBinary();
  if (handle == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "Binary semaphore creation failure" << std::endl;
#endif
  }
  xSemaphoreGive(handle);
}

BinarySemaphore& BinarySemaphore::operator=(BinarySemaphore&& s) {
  if (&s != this) {
    handle = xSemaphoreCreateBinary();
    if (handle == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "Binary semaphore creation failure" << std::endl;
#endif
    }
    xSemaphoreGive(handle);
  }
  return *this;
}

ReturnValue_t BinarySemaphore::acquire(TimeoutType timeoutType, uint32_t timeoutMs) {
  TickType_t timeout = 0;
  if (timeoutType == TimeoutType::POLLING) {
    timeout = 0;
  } else if (timeoutType == TimeoutType::WAITING) {
    timeout = pdMS_TO_TICKS(timeoutMs);
  } else {
    timeout = portMAX_DELAY;
  }
  return acquireWithTickTimeout(timeoutType, timeout);
}

ReturnValue_t BinarySemaphore::acquireWithTickTimeout(TimeoutType timeoutType,
                                                      TickType_t timeoutTicks) {
  if (handle == nullptr) {
    return SemaphoreIF::SEMAPHORE_INVALID;
  }

  BaseType_t returncode = xSemaphoreTake(handle, timeoutTicks);
  if (returncode == pdPASS) {
    return returnvalue::OK;
  } else {
    return SemaphoreIF::SEMAPHORE_TIMEOUT;
  }
}

ReturnValue_t BinarySemaphore::release() { return release(handle); }

ReturnValue_t BinarySemaphore::release(SemaphoreHandle_t semaphore) {
  if (semaphore == nullptr) {
    return SemaphoreIF::SEMAPHORE_INVALID;
  }
  BaseType_t returncode = xSemaphoreGive(semaphore);
  if (returncode == pdPASS) {
    return returnvalue::OK;
  } else {
    return SemaphoreIF::SEMAPHORE_NOT_OWNED;
  }
}

uint8_t BinarySemaphore::getSemaphoreCounter() const { return uxSemaphoreGetCount(handle); }

SemaphoreHandle_t BinarySemaphore::getSemaphore() { return handle; }

// Be careful with the stack size here. This is called from an ISR!
ReturnValue_t BinarySemaphore::releaseFromISR(SemaphoreHandle_t semaphore,
                                              BaseType_t* higherPriorityTaskWoken) {
  if (semaphore == nullptr) {
    return SemaphoreIF::SEMAPHORE_INVALID;
  }
  BaseType_t returncode = xSemaphoreGiveFromISR(semaphore, higherPriorityTaskWoken);
  if (returncode == pdPASS) {
    return returnvalue::OK;
  } else {
    return SemaphoreIF::SEMAPHORE_NOT_OWNED;
  }
}

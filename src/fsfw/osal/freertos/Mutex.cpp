#include "fsfw/osal/freertos/Mutex.h"

#include "fsfw/serviceinterface/ServiceInterface.h"

Mutex::Mutex() {
  handle = xSemaphoreCreateMutex();
  if (handle == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "Mutex::Mutex(FreeRTOS): Creation failure" << std::endl;
#endif
  }
}

Mutex::~Mutex() {
  if (handle != nullptr) {
    vSemaphoreDelete(handle);
  }
}

ReturnValue_t Mutex::lockMutex(TimeoutType timeoutType, uint32_t timeoutMs) {
  if (handle == nullptr) {
    return MutexIF::MUTEX_NOT_FOUND;
  }
  // If the timeout type is BLOCKING, this will be the correct value.
  uint32_t timeout = portMAX_DELAY;
  if (timeoutType == TimeoutType::POLLING) {
    timeout = 0;
  } else if (timeoutType == TimeoutType::WAITING) {
    timeout = pdMS_TO_TICKS(timeoutMs);
  }

  BaseType_t returncode = xSemaphoreTake(handle, timeout);
  if (returncode == pdPASS) {
    return returnvalue::OK;
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
    return returnvalue::OK;
  } else {
    return MutexIF::CURR_THREAD_DOES_NOT_OWN_MUTEX;
  }
}

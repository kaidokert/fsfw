#include "fsfw/osal/freertos/CountingSemaphUsingTask.h"

#include "fsfw/osal/freertos/TaskManagement.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

#if (tskKERNEL_VERSION_MAJOR == 8 && tskKERNEL_VERSION_MINOR > 2) || tskKERNEL_VERSION_MAJOR > 8

CountingSemaphoreUsingTask::CountingSemaphoreUsingTask(const uint8_t maxCount, uint8_t initCount)
    : maxCount(maxCount) {
  if (initCount > maxCount) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "CountingSemaphoreUsingTask: Max count bigger than "
                  "intial cout. Setting initial count to max count."
               << std::endl;
#endif
    initCount = maxCount;
  }

  handle = TaskManagement::getCurrentTaskHandle();
  if (handle == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "CountingSemaphoreUsingTask: Could not retrieve task "
                  "handle. Please ensure the constructor was called inside a "
                  "task."
               << std::endl;
#endif
  }

  uint32_t oldNotificationValue;
  xTaskNotifyAndQuery(handle, 0, eSetValueWithOverwrite, &oldNotificationValue);
  if (oldNotificationValue != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "CountinSemaphoreUsingTask: Semaphore initiated but "
                    "current notification value is not 0. Please ensure the "
                    "notification value is not used for other purposes!"
                 << std::endl;
#endif
  }
  for (int i = 0; i < initCount; i++) {
    xTaskNotifyGive(handle);
  }
}

CountingSemaphoreUsingTask::~CountingSemaphoreUsingTask() {
  // Clear notification value on destruction.
  // If this is not desired, don't call the destructor
  // (or implement a boolean which disables the reset)
  xTaskNotifyAndQuery(handle, 0, eSetValueWithOverwrite, nullptr);
}

ReturnValue_t CountingSemaphoreUsingTask::acquire(TimeoutType timeoutType, uint32_t timeoutMs) {
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

ReturnValue_t CountingSemaphoreUsingTask::acquireWithTickTimeout(TimeoutType timeoutType,
                                                                 TickType_t timeoutTicks) {
  // Decrement notfication value without resetting it.
  BaseType_t oldCount = ulTaskNotifyTake(pdFALSE, timeoutTicks);
  if (getSemaphoreCounter() == oldCount - 1) {
    return returnvalue::OK;
  } else {
    return SemaphoreIF::SEMAPHORE_TIMEOUT;
  }
}

ReturnValue_t CountingSemaphoreUsingTask::release() {
  if (getSemaphoreCounter() == maxCount) {
    return SemaphoreIF::SEMAPHORE_NOT_OWNED;
  }
  return release(handle);
}

ReturnValue_t CountingSemaphoreUsingTask::release(TaskHandle_t taskToNotify) {
  BaseType_t returncode = xTaskNotifyGive(taskToNotify);
  if (returncode == pdPASS) {
    return returnvalue::OK;
  } else {
    // This should never happen.
    return returnvalue::FAILED;
  }
}

uint8_t CountingSemaphoreUsingTask::getSemaphoreCounter() const {
  uint32_t notificationValue = 0;
  xTaskNotifyAndQuery(handle, 0, eNoAction, &notificationValue);
  return notificationValue;
}

TaskHandle_t CountingSemaphoreUsingTask::getTaskHandle() { return handle; }

ReturnValue_t CountingSemaphoreUsingTask::releaseFromISR(TaskHandle_t taskToNotify,
                                                         BaseType_t* higherPriorityTaskWoken) {
  vTaskNotifyGiveFromISR(taskToNotify, higherPriorityTaskWoken);
  return returnvalue::OK;
}

uint8_t CountingSemaphoreUsingTask::getSemaphoreCounterFromISR(
    TaskHandle_t task, BaseType_t* higherPriorityTaskWoken) {
  uint32_t notificationValue;
  xTaskNotifyAndQueryFromISR(task, 0, eNoAction, &notificationValue, higherPriorityTaskWoken);
  return notificationValue;
}

uint8_t CountingSemaphoreUsingTask::getMaxCount() const { return maxCount; }

#endif

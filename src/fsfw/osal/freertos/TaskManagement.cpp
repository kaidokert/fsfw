#include "fsfw/osal/freertos/TaskManagement.h"

void TaskManagement::vRequestContextSwitchFromTask() { vTaskDelay(0); }

void TaskManagement::requestContextSwitch(CallContext callContext = CallContext::TASK) {
  if (callContext == CallContext::ISR) {
    // This function depends on the partmacro.h definition for the specific device
    vRequestContextSwitchFromISR();
  } else {
    vRequestContextSwitchFromTask();
  }
}

TaskHandle_t TaskManagement::getCurrentTaskHandle() { return xTaskGetCurrentTaskHandle(); }

size_t TaskManagement::getTaskStackHighWatermark(TaskHandle_t task) {
  return uxTaskGetStackHighWaterMark(task) * sizeof(StackType_t);
}

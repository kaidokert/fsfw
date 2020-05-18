#include <framework/osal/FreeRTOS/TaskManagement.h>

void TaskManagement::requestContextSwitchFromTask() {
	vTaskDelay(0);
}

void TaskManagement::requestContextSwitch(
		CallContext callContext = CallContext::task) {
	if(callContext == CallContext::isr) {
		// This function depends on the partmacro.h definition for the specific device
		requestContextSwitchFromISR();
	} else {
		requestContextSwitchFromTask();
	}
}

TaskHandle_t TaskManagement::getCurrentTaskHandle() {
	return xTaskGetCurrentTaskHandle();
}

configSTACK_DEPTH_TYPE TaskManagement::getTaskStackHighWatermark() {
	return uxTaskGetStackHighWaterMark(TaskManagement::getCurrentTaskHandle());
}

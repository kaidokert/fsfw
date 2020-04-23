/**
 * @file TaskManagement.cpp
 *
 * @date 26.02.2020
 *
 */
#include <framework/osal/FreeRTOS/TaskManagement.h>

extern "C" {
#include "FreeRTOS.h"
#include "task.h"
}

void TaskManagement::requestContextSwitchFromTask() {
	vTaskDelay(0);
}

void TaskManagement::requestContextSwitch(CallContext callContext = CallContext::task) {
	if(callContext == CallContext::isr) {
		// This function depends on the partmacro.h definition for the specific device
		requestContextSwitchFromISR();
	} else {
		requestContextSwitchFromTask();
	}
}




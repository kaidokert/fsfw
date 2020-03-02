/**
 * @file TaskManagement.cpp
 *
 * @date 26.02.2020
 *
 */
#include <framework/osal/FreeRTOS/TaskManagement.h>
#include <FreeRTOS.h>
#include "portmacro.h"
#include "task.h"

void TaskManagement::requestContextSwitchFromTask() {
	vTaskDelay(0);
}

void TaskManagement::requestContextSwitch(CallContext callContext = CallContext::task) {
	if(callContext == CallContext::isr) {
		// This function depends on the partmacro.h definition for the specific device
		portYIELD_FROM_ISR();
	} else {
		requestContextSwitchFromTask();
	}
}




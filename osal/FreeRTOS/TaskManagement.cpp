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

void TaskManagement::requestContextSwitch(SystemContext callContext = SystemContext::task_context) {
	if(callContext == SystemContext::isr_context) {
		// This function depends on the partmacro.h definition for the specific device
		portYIELD_FROM_ISR();
	} else {
		requestContextSwitchFromTask();
	}
}




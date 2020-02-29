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

/**
 * TODO: This stuff is hardware and architecture and mission dependant...
 * 	     Some FreeRTOS implementations might be able to determine their own task context for example.
 * 	     If not ISRs are used, or task preemption is enabled, some of this stuff might
 * 	     not be necessary anyway. Maybe there is a better solution?
 */
void requestContextSwitchFromTask() {
	vTaskDelay(0);
}

void requestContextSwitch(SystemContext callContext) {
	if(callContext == SystemContext::isr_context) {
		// This function depends on the partmacro.h definition for the specific device
		portYIELD_FROM_ISR();
	} else {
		requestContextSwitchFromTask();
	}
}


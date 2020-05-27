#include <framework/osal/FreeRTOS/CountingSemaphUsingTask.h>
#include <framework/osal/FreeRTOS/TaskManagement.h>

CountingSemaphoreUsingTask::CountingSemaphoreUsingTask(uint8_t count,
		uint8_t initCount):
		count(count), initCount(initCount) {
	handle = TaskManagement::getCurrentTaskHandle();
}

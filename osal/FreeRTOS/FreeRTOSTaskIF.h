#ifndef FRAMEWORK_OSAL_FREERTOS_FREERTOSTASKIF_H_
#define FRAMEWORK_OSAL_FREERTOS_FREERTOSTASKIF_H_

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class FreeRTOSTaskIF {
public:
	virtual~ FreeRTOSTaskIF() {}
	virtual TaskHandle_t getTaskHandle() = 0;
};

#endif /* FRAMEWORK_OSAL_FREERTOS_FREERTOSTASKIF_H_ */

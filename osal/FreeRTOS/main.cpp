//entry point into "bsp"
void init(void);

#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include "task.h"


void initTask(void *parameters) {
	init();
}

int main(void) {

	if ( pdPASS
			!= xTaskCreate(initTask, "init", 512, NULL,
					configMAX_PRIORITIES - 1, NULL)) {
		//print_uart0("Could not create task1\r\n");
	}

	vTaskStartScheduler();

	//Scheduler should never return

	//print_uart0("This is bad\n");

	for (;;)
		;

	return 0;
}

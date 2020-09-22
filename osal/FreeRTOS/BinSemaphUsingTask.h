#ifndef FRAMEWORK_OSAL_FREERTOS_BINSEMAPHUSINGTASK_H_
#define FRAMEWORK_OSAL_FREERTOS_BINSEMAPHUSINGTASK_H_

#include "../../returnvalues/HasReturnvaluesIF.h"
#include "../../tasks/SemaphoreIF.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/**
 * @brief 	Binary Semaphore implementation using the task notification value.
 * 			The notification value should therefore not be used
 * 			for other purposes.
 * @details
 * Additional information: https://www.freertos.org/RTOS-task-notifications.html
 * and general semaphore documentation.
 */
class BinarySemaphoreUsingTask: public SemaphoreIF,
		public HasReturnvaluesIF {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::SEMAPHORE_IF;

	//! @brief Default ctor
	BinarySemaphoreUsingTask();
	//! @brief Default dtor
	virtual~ BinarySemaphoreUsingTask();

	ReturnValue_t acquire(TimeoutType timeoutType = TimeoutType::BLOCKING,
	        uint32_t timeoutMs = portMAX_DELAY) override;
	ReturnValue_t release() override;
	uint8_t getSemaphoreCounter() const override;
	static uint8_t getSemaphoreCounter(TaskHandle_t taskHandle);
	static uint8_t getSemaphoreCounterFromISR(TaskHandle_t taskHandle,
			BaseType_t* higherPriorityTaskWoken);

	/**
	 * Same as acquire() with timeout in FreeRTOS ticks.
	 * @param timeoutTicks
	 * @return - @c RETURN_OK on success
	 *         - @c RETURN_FAILED on failure
	 */
	ReturnValue_t acquireWithTickTimeout(
	        TimeoutType timeoutType = TimeoutType::BLOCKING,
	        TickType_t timeoutTicks = portMAX_DELAY);

	/**
	 * Get handle to the task related to the semaphore.
	 * @return
	 */
	TaskHandle_t getTaskHandle();

	 /**
	 * Wrapper function to give back semaphore from handle
	 * @param semaphore
	 * @return - @c RETURN_OK on success
	 *         - @c RETURN_FAILED on failure
	 */
	static ReturnValue_t release(TaskHandle_t taskToNotify);

	/**
	 * Wrapper function to give back semaphore from handle when called from an ISR
	 * @param semaphore
	 * @param higherPriorityTaskWoken This will be set to pdPASS if a task with
	 * a higher priority was unblocked. A context switch should be requested
	 * from an ISR if this is the case (see TaskManagement functions)
	 * @return - @c RETURN_OK on success
	 *         - @c RETURN_FAILED on failure
	 */
	static ReturnValue_t releaseFromISR(TaskHandle_t taskToNotify,
				BaseType_t * higherPriorityTaskWoken);

protected:
	TaskHandle_t handle;
};

#endif /* FRAMEWORK_OSAL_FREERTOS_BINSEMAPHUSINGTASK_H_ */

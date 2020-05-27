#ifndef FRAMEWORK_OSAL_FREERTOS_BINSEMAPHUSINGTASK_H_
#define FRAMEWORK_OSAL_FREERTOS_BINSEMAPHUSINGTASK_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/tasks/SemaphoreIF.h>

extern "C" {
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
}

/**
 * @brief 	Binary Semaphore implementation using Task Notifications
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

	ReturnValue_t acquire(uint32_t timeoutMs =
	        SemaphoreIF::NO_TIMEOUT) override;
	ReturnValue_t release() override;
	uint8_t getSemaphoreCounter() const override;

	/**
	 * Take the binary semaphore.
	 * If the semaphore has already been taken, the task will be blocked
	 * for a maximum of #timeoutMs or until the semaphore is given back,
	 * for example by an ISR or another task.
	 * @param timeoutMs
	 * @return -@c RETURN_OK on success
	 *         -@c RETURN_FAILED on failure
	 */
	ReturnValue_t takeBinarySemaphore(uint32_t timeoutMs =
	       	   SemaphoreIF::NO_TIMEOUT);

	/**
	 * Same as lockBinarySemaphore() with timeout in FreeRTOS ticks.
	 * @param timeoutTicks
	 * @return - @c RETURN_OK on success
	 *         - @c RETURN_FAILED on failure
	 */
	ReturnValue_t takeBinarySemaphoreTickTimeout(TickType_t timeoutTicks =
	        SemaphoreIF::NO_TIMEOUT);

	/**
	 * Give back the binary semaphore
	 * @return - @c RETURN_OK on success
	 *         - @c RETURN_FAILED on failure
	 */
	ReturnValue_t giveBinarySemaphore();

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
	static ReturnValue_t giveBinarySemaphore(TaskHandle_t taskToNotify);

	/**
	 * Wrapper function to give back semaphore from handle when called from an ISR
	 * @param semaphore
	 * @param higherPriorityTaskWoken This will be set to pdPASS if a task with a higher priority
	 *        was unblocked
	 * @return - @c RETURN_OK on success
	 *         - @c RETURN_FAILED on failure
	 */
	static ReturnValue_t giveBinarySemaphoreFromISR(TaskHandle_t taskToNotify,
				BaseType_t * higherPriorityTaskWoken);

	static uint8_t getSemaphoreCounterFromISR(TaskHandle_t taskHandle);

protected:
	TaskHandle_t handle;
	// This boolean is required to track whether the semaphore is locked
	// or unlocked.
	bool locked;
};



#endif /* FRAMEWORK_OSAL_FREERTOS_BINSEMAPHUSINGTASK_H_ */

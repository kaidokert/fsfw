#ifndef FRAMEWORK_OSAL_FREERTOS_COUNTINGSEMAPHUSINGTASK_H_
#define FRAMEWORK_OSAL_FREERTOS_COUNTINGSEMAPHUSINGTASK_H_

#include <framework/osal/FreeRTOS/CountingSemaphUsingTask.h>
#include <framework/tasks/SemaphoreIF.h>

extern "C" {
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
}

/**
 * @brief 	Couting Semaphore implementation which uses the notification value
 * 			of the task. The notification value should therefore not be used
 * 			for other purposes
 * @details
 *  Additional information: https://www.freertos.org/RTOS-task-notifications.html
 *  and general semaphore documentation.
 */
class CountingSemaphoreUsingTask: public SemaphoreIF {
public:
	CountingSemaphoreUsingTask(const uint8_t maxCount, uint8_t initCount);
	virtual ~CountingSemaphoreUsingTask();

	ReturnValue_t acquire(uint32_t timeoutMs = SemaphoreIF::NO_TIMEOUT) override;
	ReturnValue_t release() override;
	uint8_t getSemaphoreCounter() const override;
	static uint8_t getSemaphoreCounterFromISR(TaskHandle_t task);
	/**
	 * Acquire, using a timeout value in ticks
	 * @param timeoutTicks
	 * @return
	 */
	ReturnValue_t acquireWithTickTimeout(
			TickType_t timeoutTicks= SemaphoreIF::NO_TIMEOUT);

	/**
	 * Get handle to the task related to the semaphore.
	 * @return
	 */
	TaskHandle_t getTaskHandle();

	/**
	 * Release semaphore of task by supplying task handle
	 * @param taskToNotify
	 * @return
	 */
	static ReturnValue_t release(TaskHandle_t taskToNotify);
	/**
	 * Release seamphore of a task from an ISR.
	 * @param taskToNotify
	 * @return
	 */
	static ReturnValue_t releaseFromISR(TaskHandle_t taskToNotify,
			BaseType_t* higherPriorityTaskWoken);

private:
	TaskHandle_t handle;
	const uint8_t maxCount;
	uint8_t currentCount = 0;
};

#endif /* FRAMEWORK_OSAL_FREERTOS_COUNTINGSEMAPHUSINGTASK_H_ */

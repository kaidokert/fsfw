#ifndef FSFW_OSAL_FREERTOS_COUNTINGSEMAPHUSINGTASK_H_
#define FSFW_OSAL_FREERTOS_COUNTINGSEMAPHUSINGTASK_H_

#include "CountingSemaphUsingTask.h"
#include "FreeRTOS.h"
#include "fsfw/tasks/SemaphoreIF.h"
#include "task.h"

#if (tskKERNEL_VERSION_MAJOR == 8 && tskKERNEL_VERSION_MINOR > 2) || tskKERNEL_VERSION_MAJOR > 8

/**
 * @brief 	Couting Semaphore implementation which uses the notification value
 * 			of the task. The notification value should therefore not be used
 * 			for other purposes.
 * @details
 *  Additional information: https://www.freertos.org/RTOS-task-notifications.html
 *  and general semaphore documentation.
 *  This semaphore is bound to the task it is created in!
 *  Take care of calling this function with the correct executing task,
 *  (for example in the initializeAfterTaskCreation() function).
 */
class CountingSemaphoreUsingTask : public SemaphoreIF {
 public:
  CountingSemaphoreUsingTask(const uint8_t maxCount, uint8_t initCount);
  virtual ~CountingSemaphoreUsingTask();

  /**
   * Acquire the counting semaphore.
   * If no semaphores are available, the task will be blocked
   * for a maximum of #timeoutMs or until one is given back,
   * for example by an ISR or another task.
   * @param timeoutMs
   * @return -@c returnvalue::OK on success
   *         -@c SemaphoreIF::SEMAPHORE_TIMEOUT on timeout
   */
  ReturnValue_t acquire(TimeoutType timeoutType = TimeoutType::BLOCKING,
                        uint32_t timeoutMs = portMAX_DELAY) override;

  /**
   * Release a semaphore, increasing the number of available counting
   * semaphores up to the #maxCount value.
   * @return -@c returnvalue::OK on success
   *         -@c SemaphoreIF::SEMAPHORE_NOT_OWNED if #maxCount semaphores are
   *         	already available.
   */
  ReturnValue_t release() override;

  uint8_t getSemaphoreCounter() const override;
  /**
   * Get the semaphore counter from an ISR.
   * @param task
   * @param higherPriorityTaskWoken This will be set to pdPASS if a task with
   * a higher priority was unblocked. A context switch should be requested
   * from an ISR if this is the case (see TaskManagement functions)
   * @return
   */
  static uint8_t getSemaphoreCounterFromISR(TaskHandle_t task, BaseType_t* higherPriorityTaskWoken);

  /**
   * Acquire with a timeout value in ticks
   * @param timeoutTicks
   * @return -@c returnvalue::OK on success
   *         -@c SemaphoreIF::SEMAPHORE_TIMEOUT on timeout
   */
  ReturnValue_t acquireWithTickTimeout(TimeoutType timeoutType = TimeoutType::BLOCKING,
                                       TickType_t timeoutTicks = portMAX_DELAY);

  /**
   * Get handle to the task related to the semaphore.
   * @return
   */
  TaskHandle_t getTaskHandle();

  /**
   * Release semaphore of task by supplying task handle
   * @param taskToNotify
   * @return -@c returnvalue::OK on success
   *         -@c SemaphoreIF::SEMAPHORE_NOT_OWNED if #maxCount semaphores are
   *         	already available.
   */
  static ReturnValue_t release(TaskHandle_t taskToNotify);
  /**
   * Release seamphore of a task from an ISR.
   * @param taskToNotify
   * @param higherPriorityTaskWoken This will be set to pdPASS if a task with
   * a higher priority was unblocked. A context switch should be requested
   * from an ISR if this is the case (see TaskManagement functions)
   * @return -@c returnvalue::OK on success
   *         -@c SemaphoreIF::SEMAPHORE_NOT_OWNED if #maxCount semaphores are
   *         	already available.
   */
  static ReturnValue_t releaseFromISR(TaskHandle_t taskToNotify,
                                      BaseType_t* higherPriorityTaskWoken);

  uint8_t getMaxCount() const;

 private:
  TaskHandle_t handle;
  const uint8_t maxCount;
};

#endif /* (tskKERNEL_VERSION_MAJOR == 8 && tskKERNEL_VERSION_MINOR > 2) || \
    tskKERNEL_VERSION_MAJOR > 8 */

#endif /* FSFW_OSAL_FREERTOS_COUNTINGSEMAPHUSINGTASK_H_ */

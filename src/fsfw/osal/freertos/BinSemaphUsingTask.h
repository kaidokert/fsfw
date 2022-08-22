#ifndef FSFW_OSAL_FREERTOS_BINSEMAPHUSINGTASK_H_
#define FSFW_OSAL_FREERTOS_BINSEMAPHUSINGTASK_H_

#include "FreeRTOS.h"
#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw/tasks/SemaphoreIF.h"
#include "task.h"

#if (tskKERNEL_VERSION_MAJOR == 8 && tskKERNEL_VERSION_MINOR > 2) || tskKERNEL_VERSION_MAJOR > 8

/**
 * @brief 	Binary Semaphore implementation using the task notification value.
 * 			The notification value should therefore not be used
 * 			for other purposes!
 * @details
 * Additional information: https://www.freertos.org/RTOS-task-notifications.html
 * and general semaphore documentation.
 * This semaphore is bound to the task it is created in!
 * Take care of building this class with the correct executing task,
 * (for example in the initializeAfterTaskCreation() function) or
 * by calling refreshTaskHandle() with the correct executing task.
 */
class BinarySemaphoreUsingTask : public SemaphoreIF {
 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::SEMAPHORE_IF;

  //! @brief Default ctor
  BinarySemaphoreUsingTask();
  //! @brief Default dtor
  virtual ~BinarySemaphoreUsingTask();

  /**
   * This function can be used to get the correct task handle from the
   * currently executing task.
   *
   * This is required because the task notification value will be used
   * as a binary semaphore, and the semaphore might be created by another
   * task.
   */
  void refreshTaskHandle();

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
   * @return - @c returnvalue::OK on success
   *         - @c returnvalue::FAILED on failure
   */
  ReturnValue_t acquireWithTickTimeout(TimeoutType timeoutType = TimeoutType::BLOCKING,
                                       TickType_t timeoutTicks = portMAX_DELAY);

  /**
   * Get handle to the task related to the semaphore.
   * @return
   */
  TaskHandle_t getTaskHandle();

  /**
   * Wrapper function to give back semaphore from handle
   * @param semaphore
   * @return - @c returnvalue::OK on success
   *         - @c returnvalue::FAILED on failure
   */
  static ReturnValue_t release(TaskHandle_t taskToNotify);

  /**
   * Wrapper function to give back semaphore from handle when called from an ISR
   * @param semaphore
   * @param higherPriorityTaskWoken This will be set to pdPASS if a task with
   * a higher priority was unblocked. A context switch should be requested
   * from an ISR if this is the case (see TaskManagement functions)
   * @return - @c returnvalue::OK on success
   *         - @c returnvalue::FAILED on failure
   */
  static ReturnValue_t releaseFromISR(TaskHandle_t taskToNotify,
                                      BaseType_t* higherPriorityTaskWoken);

 protected:
  TaskHandle_t handle;
};

#endif /* (tskKERNEL_VERSION_MAJOR == 8 && tskKERNEL_VERSION_MINOR > 2) || \
    tskKERNEL_VERSION_MAJOR > 8 */

#endif /* FSFW_OSAL_FREERTOS_BINSEMAPHUSINGTASK_H_ */

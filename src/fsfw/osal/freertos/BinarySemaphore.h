#ifndef FSFW_OSAL_FREERTOS_BINARYSEMPAHORE_H_
#define FSFW_OSAL_FREERTOS_BINARYSEMPAHORE_H_

#include "FreeRTOS.h"
#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw/tasks/SemaphoreIF.h"
#include "semphr.h"

/**
 * @brief 	OS Tool to achieve synchronization of between tasks or between
 * 			task and ISR. The default semaphore implementation creates a
 * 			binary semaphore, which can only be taken once.
 * @details
 * Documentation: https://www.freertos.org/Embedded-RTOS-Binary-Semaphores.html
 *
 * Please note that if the semaphore implementation is only related to
 * the synchronization of one task, the new task notifications can be used,
 * also see the BinSemaphUsingTask and CountingSemaphUsingTask classes.
 * These use the task notification value instead of a queue and are
 * faster and more efficient.
 *
 * @author 	R. Mueller
 * @ingroup osal
 */
class BinarySemaphore : public SemaphoreIF {
 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::SEMAPHORE_IF;

  //! @brief Default ctor
  BinarySemaphore();
  //! @brief Copy ctor, deleted explicitely.
  BinarySemaphore(const BinarySemaphore &) = delete;
  //! @brief Copy assignment, deleted explicitely.
  BinarySemaphore &operator=(const BinarySemaphore &) = delete;
  //! @brief Move ctor
  BinarySemaphore(BinarySemaphore &&);
  //! @brief Move assignment
  BinarySemaphore &operator=(BinarySemaphore &&);
  //! @brief Destructor
  virtual ~BinarySemaphore();

  uint8_t getSemaphoreCounter() const override;

  /**
   * Take the binary semaphore.
   * If the semaphore has already been taken, the task will be blocked
   * for a maximum of #timeoutMs or until the semaphore is given back,
   * for example by an ISR or another task.
   * @param timeoutMs
   * @return -@c returnvalue::OK on success
   *         -@c SemaphoreIF::SEMAPHORE_TIMEOUT on timeout
   */
  ReturnValue_t acquire(TimeoutType timeoutType = TimeoutType::BLOCKING,
                        uint32_t timeoutMs = portMAX_DELAY) override;

  /**
   * Same as lockBinarySemaphore() with timeout in FreeRTOS ticks.
   * @param timeoutTicks
   * @return -@c returnvalue::OK on success
   *         -@c SemaphoreIF::SEMAPHORE_TIMEOUT on timeout
   */
  ReturnValue_t acquireWithTickTimeout(TimeoutType timeoutType = TimeoutType::BLOCKING,
                                       TickType_t timeoutTicks = portMAX_DELAY);

  /**
   * Release the binary semaphore.
   * @return -@c returnvalue::OK on success
   *         -@c SemaphoreIF::SEMAPHORE_NOT_OWNED if the semaphores is
   *         	already available.
   */
  ReturnValue_t release() override;

  /**
   * Get Handle to the semaphore.
   * @return
   */
  SemaphoreHandle_t getSemaphore();

  /**
   * Wrapper function to give back semaphore from handle
   * @param semaphore
   * @return -@c returnvalue::OK on success
   *         -@c SemaphoreIF::SEMAPHORE_NOT_OWNED if the semaphores is
   *         	already available.
   */
  static ReturnValue_t release(SemaphoreHandle_t semaphore);

  /**
   * Wrapper function to give back semaphore from handle when called from an ISR
   * @param semaphore
   * @param higherPriorityTaskWoken This will be set to pdPASS if a task with
   * a higher priority was unblocked. A context switch from an ISR should
   * then be requested (see TaskManagement functions)
   * @return -@c returnvalue::OK on success
   *         -@c SemaphoreIF::SEMAPHORE_NOT_OWNED if the semaphores is
   *         	already available.
   */
  static ReturnValue_t releaseFromISR(SemaphoreHandle_t semaphore,
                                      BaseType_t *higherPriorityTaskWoken);

 protected:
  SemaphoreHandle_t handle;
};

#endif /* FSFW_OSAL_FREERTOS_BINARYSEMPAHORE_H_ */

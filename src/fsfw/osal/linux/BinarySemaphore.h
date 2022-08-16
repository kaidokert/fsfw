#ifndef FRAMEWORK_OSAL_LINUX_BINARYSEMPAHORE_H_
#define FRAMEWORK_OSAL_LINUX_BINARYSEMPAHORE_H_

#include "../../returnvalues/returnvalue.h"
#include "../../tasks/SemaphoreIF.h"

extern "C" {
#include <semaphore.h>
}

/**
 * @brief 	OS Tool to achieve synchronization of between tasks or between
 * 			task and ISR. The default semaphore implementation creates a
 * 			binary semaphore, which can only be taken once.
 * @details
 * See: http://www.man7.org/linux/man-pages/man7/sem_overview.7.html
 * @author 	R. Mueller
 * @ingroup osal
 */
class BinarySemaphore : public SemaphoreIF {
 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::SEMAPHORE_IF;

  //! @brief Default ctor
  BinarySemaphore();
  //! @brief Copy ctor, deleted explicitely.
  BinarySemaphore(const BinarySemaphore&) = delete;
  //! @brief Copy assignment, deleted explicitely.
  BinarySemaphore& operator=(const BinarySemaphore&) = delete;
  //! @brief Move ctor
  BinarySemaphore(BinarySemaphore&&);
  //! @brief Move assignment
  BinarySemaphore& operator=(BinarySemaphore&&);
  //! @brief Destructor
  virtual ~BinarySemaphore();

  void initSemaphore(uint8_t initCount = 1);

  uint8_t getSemaphoreCounter() const override;
  static uint8_t getSemaphoreCounter(sem_t* handle);

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
                        uint32_t timeoutMs = 0) override;

  /**
   * Release the binary semaphore.
   * @return -@c returnvalue::OK on success
   *         -@c SemaphoreIF::SEMAPHORE_NOT_OWNED if the semaphores is
   *         	already available.
   */
  virtual ReturnValue_t release() override;
  /**
   * This static function can be used to release a semaphore  by providing
   * its handle.
   * @param handle
   * @return
   */
  static ReturnValue_t release(sem_t* handle);

  /** Checks the validity of the semaphore count against a specified
   * known maxCount
   * @param handle
   * @param maxCount
   * @return
   */
  static ReturnValue_t checkCount(sem_t* handle, uint8_t maxCount);

 protected:
  sem_t handle;
  static constexpr const char* CLASS_NAME = "BinarySemaphore";
};

#endif /* FRAMEWORK_OSAL_FREERTOS_BINARYSEMPAHORE_H_ */

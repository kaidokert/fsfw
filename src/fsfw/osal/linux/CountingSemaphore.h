#ifndef FRAMEWORK_OSAL_LINUX_COUNTINGSEMAPHORE_H_
#define FRAMEWORK_OSAL_LINUX_COUNTINGSEMAPHORE_H_
#include "../../osal/linux/BinarySemaphore.h"

/**
 * @brief 	Counting semaphores, which can be acquired more than once.
 * @details
 * See: https://www.freertos.org/CreateCounting.html
 * API of counting semaphores is almost identical to binary semaphores,
 * so we just inherit from binary semaphore and provide the respective
 * constructors.
 */
class CountingSemaphore : public BinarySemaphore {
 public:
  CountingSemaphore(const uint8_t maxCount, uint8_t initCount);
  //! @brief Copy ctor, disabled
  CountingSemaphore(const CountingSemaphore&) = delete;
  //! @brief Copy assignment, disabled
  CountingSemaphore& operator=(const CountingSemaphore&) = delete;
  //! @brief Move ctor
  CountingSemaphore(CountingSemaphore&&);
  //! @brief Move assignment
  CountingSemaphore& operator=(CountingSemaphore&&);

  ReturnValue_t release() override;
  static ReturnValue_t release(sem_t* sem);
  /* Same API as binary semaphore otherwise. acquire() can be called
   * until there are not semaphores left and release() can be called
   * until maxCount is reached. */

  uint8_t getMaxCount() const;

 private:
  const uint8_t maxCount;
  uint8_t initCount = 0;
};

#endif /* FRAMEWORK_OSAL_FREERTOS_COUNTINGSEMAPHORE_H_ */

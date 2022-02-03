#ifndef FSFW_OSAL_HOSTED_MUTEX_H_
#define FSFW_OSAL_HOSTED_MUTEX_H_

#include <mutex>

#include "fsfw/ipc/MutexIF.h"

/**
 * @brief OS component to implement MUTual EXclusion
 *
 * @details
 * Mutexes are binary semaphores which include a priority inheritance mechanism.
 * Documentation: https://www.freertos.org/Real-time-embedded-RTOS-mutexes.html
 * @ingroup osal
 */
class Mutex : public MutexIF {
 public:
  Mutex();
  ReturnValue_t lockMutex(TimeoutType timeoutType = TimeoutType::BLOCKING,
                          uint32_t timeoutMs = 0) override;
  ReturnValue_t unlockMutex() override;

  std::timed_mutex* getMutexHandle();

 private:
  std::timed_mutex mutex;
};

#endif /* FSFW_OSAL_HOSTED_MUTEX_H_ */

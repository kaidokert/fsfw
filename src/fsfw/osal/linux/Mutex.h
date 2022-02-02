#ifndef FSFW_OSAL_LINUX_MUTEX_H_
#define FSFW_OSAL_LINUX_MUTEX_H_

#include <pthread.h>

#include "../../ipc/MutexIF.h"

class Mutex : public MutexIF {
 public:
  Mutex();
  virtual ~Mutex();
  virtual ReturnValue_t lockMutex(TimeoutType timeoutType, uint32_t timeoutMs);
  virtual ReturnValue_t unlockMutex();

 private:
  pthread_mutex_t mutex;
  static uint8_t count;
};

#endif /* OS_RTEMS_MUTEX_H_ */

#ifndef FRAMEWORK_IPC_MUTEXGUARD_H_
#define FRAMEWORK_IPC_MUTEXGUARD_H_

#include <fmt/core.h>

#include "fsfw/ipc/MutexIF.h"
#include "fsfw/serviceinterface.h"

class MutexGuard {
 public:
  MutexGuard(MutexIF* mutex, MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::BLOCKING,
             uint32_t timeoutMs = 0)
      : internalMutex(mutex) {
    if (mutex == nullptr) {
      // It's tricky to use the error functions defined in the service interface
      // because those functions require the mutex guard themselves
      fmt::print("ERROR | Passed mutex is invalid\n");
      return;
    }
    result = mutex->lockMutex(timeoutType, timeoutMs);
    if (result == MutexIF::MUTEX_TIMEOUT) {
      fmt::print("ERROR | Lock of mutex failed with timeout of {} milliseconds\n", timeoutMs);
    } else if (result != HasReturnvaluesIF::RETURN_OK) {
      fmt::print("ERROR | Lock of Mutex failed with code {}\n", result);
    }
  }

  ReturnValue_t getLockResult() const { return result; }

  ~MutexGuard() {
    if (internalMutex != nullptr) {
      internalMutex->unlockMutex();
    }
  }

 private:
  MutexIF* internalMutex;
  ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
};

#endif /* FRAMEWORK_IPC_MUTEXGUARD_H_ */

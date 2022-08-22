#ifndef FRAMEWORK_IPC_MUTEXGUARD_H_
#define FRAMEWORK_IPC_MUTEXGUARD_H_

#include "../serviceinterface/ServiceInterface.h"
#include "MutexFactory.h"

class MutexGuard {
 public:
  MutexGuard(MutexIF* mutex, MutexIF::TimeoutType timeoutType = MutexIF::TimeoutType::BLOCKING,
             uint32_t timeoutMs = 0)
      : internalMutex(mutex) {
    if (mutex == nullptr) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "MutexGuard: Passed mutex is invalid!" << std::endl;
#else
      sif::printError("MutexGuard: Passed mutex is invalid!\n");
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
      return;
    }
    result = mutex->lockMutex(timeoutType, timeoutMs);
#if FSFW_VERBOSE_LEVEL >= 1
    if (result == MutexIF::MUTEX_TIMEOUT) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "MutexGuard: Lock of mutex failed with timeout of " << timeoutMs
                 << " milliseconds!" << std::endl;
#else
      sif::printError("MutexGuard: Lock of mutex failed with timeout of %lu milliseconds\n",
                      timeoutMs);
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */

    } else if (result != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "MutexGuard: Lock of Mutex failed with code " << result << std::endl;
#else
      sif::printError("MutexGuard: Lock of Mutex failed with code %d\n", result);
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
    }
#else
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
  }

  ReturnValue_t getLockResult() const { return result; }

  ~MutexGuard() {
    if (internalMutex != nullptr) {
      internalMutex->unlockMutex();
    }
  }

 private:
  MutexIF* internalMutex;
  ReturnValue_t result = returnvalue::FAILED;
};

#endif /* FRAMEWORK_IPC_MUTEXGUARD_H_ */

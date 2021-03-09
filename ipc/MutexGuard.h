#ifndef FRAMEWORK_IPC_MUTEXHELPER_H_
#define FRAMEWORK_IPC_MUTEXHELPER_H_

#include "MutexFactory.h"
#include "../serviceinterface/ServiceInterface.h"

class MutexGuard {
public:
    MutexGuard(MutexIF* mutex, MutexIF::TimeoutType timeoutType =
            MutexIF::TimeoutType::BLOCKING, uint32_t timeoutMs = 0):
            internalMutex(mutex) {
        if(mutex == nullptr) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "MutexHelper: Passed mutex is invalid!" << std::endl;
#else
            sif::printError("MutexHelper: Passed mutex is invalid!\n");
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
            return;
        }
        result = mutex->lockMutex(timeoutType,
                timeoutMs);
#if FSFW_VERBOSE_LEVEL >= 1
        if(result == MutexIF::MUTEX_TIMEOUT) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "MutexHelper: Lock of mutex failed with timeout of "
                    << timeoutMs << " milliseconds!" << std::endl;
#else
            sif::printError("MutexHelper: Lock of mutex failed with timeout of %lu milliseconds\n",
                    timeoutMs);
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */

        }
        else if(result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "MutexHelper: Lock of Mutex failed with code " << result << std::endl;
#else
            sif::printError("MutexHelper: Lock of Mutex failed with code %d\n", result);
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
        }
#else
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
    }

    ReturnValue_t getLockResult() const {
        return result;
    }

    ~MutexGuard() {
        if(internalMutex != nullptr) {
            internalMutex->unlockMutex();
        }
    }
private:
    MutexIF* internalMutex;
    ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
};

#endif /* FRAMEWORK_IPC_MUTEXHELPER_H_ */

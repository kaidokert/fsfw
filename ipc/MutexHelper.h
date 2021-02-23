#ifndef FRAMEWORK_IPC_MUTEXHELPER_H_
#define FRAMEWORK_IPC_MUTEXHELPER_H_

#include "MutexFactory.h"
#include "../serviceinterface/ServiceInterface.h"

class MutexHelper {
public:
    MutexHelper(MutexIF* mutex, MutexIF::TimeoutType timeoutType =
            MutexIF::TimeoutType::BLOCKING, uint32_t timeoutMs = 0):
            internalMutex(mutex) {
        if(mutex == nullptr) {
            return;
        }
        ReturnValue_t status = mutex->lockMutex(timeoutType,
                timeoutMs);
        if(status == MutexIF::MUTEX_TIMEOUT) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "MutexHelper: Lock of mutex failed with timeout of "
                    << timeoutMs << " milliseconds!" << std::endl;
#else
            sif::printError("MutexHelper: Lock of mutex failed with timeout of %lu milliseconds\n",
                    timeoutMs);
#endif
        }
        else if(status != HasReturnvaluesIF::RETURN_OK){
#if FSFW_CPP_OSTREAM_ENABLED == 1
            sif::error << "MutexHelper: Lock of Mutex failed with code "
                    << status << std::endl;
#else
            sif::printError("MutexHelper: Lock of Mutex failed with code %d\n", status);
#endif
        }
    }

    ~MutexHelper() {
        if(internalMutex != nullptr) {
            internalMutex->unlockMutex();
        }
    }
private:
    MutexIF* internalMutex;
};

#endif /* FRAMEWORK_IPC_MUTEXHELPER_H_ */

#ifndef FRAMEWORK_IPC_MUTEXHELPER_H_
#define FRAMEWORK_IPC_MUTEXHELPER_H_

#include "MutexFactory.h"
#include "../serviceinterface/ServiceInterfaceStream.h"

class MutexHelper {
public:
	MutexHelper(MutexIF* mutex, MutexIF::TimeoutType timeoutType =
			MutexIF::TimeoutType::BLOCKING, uint32_t timeoutMs = 0) :
			internalMutex(mutex) {
		ReturnValue_t status = mutex->lockMutex(timeoutType,
		        timeoutMs);
		if(status == MutexIF::MUTEX_TIMEOUT) {
			sif::error << "MutexHelper: Lock of mutex failed with timeout of "
					<< timeoutMs << " milliseconds!" << std::endl;
		}
		else if(status != HasReturnvaluesIF::RETURN_OK){
			sif::error << "MutexHelper: Lock of Mutex failed with code " <<
					status << std::endl;
		}
	}

	~MutexHelper() {
		internalMutex->unlockMutex();
	}
private:
	MutexIF* internalMutex;
};
#endif /* FRAMEWORK_IPC_MUTEXHELPER_H_ */

#ifndef FRAMEWORK_IPC_MUTEXHELPER_H_
#define FRAMEWORK_IPC_MUTEXHELPER_H_

#include <framework/ipc/MutexFactory.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

class MutexHelper {
public:
	MutexHelper(MutexIF* mutex, uint32_t timeoutMs) :
			internalMutex(mutex) {
		ReturnValue_t status = mutex->lockMutex(timeoutMs);
		if(status != HasReturnvaluesIF::RETURN_OK){
			sif::error << "MutexHelper: Lock of Mutex failed " <<
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

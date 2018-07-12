#ifndef FRAMEWORK_IPC_MUTEXIF_H_
#define FRAMEWORK_IPC_MUTEXIF_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>

class MutexIF {
public:
	static const uint32_t NO_TIMEOUT; //!< Needs to be defined in implementation.
	virtual ~MutexIF() {}
	virtual ReturnValue_t lockMutex(uint32_t timeoutMs) = 0;
	virtual ReturnValue_t unlockMutex() = 0;
};



#endif /* FRAMEWORK_IPC_MUTEXIF_H_ */

#ifndef FRAMEWORK_OSAL_RTEMS_MUTEX_H_
#define FRAMEWORK_OSAL_RTEMS_MUTEX_H_

#include "../../ipc/MutexIF.h"
#include "RtemsBasic.h"

class Mutex : public MutexIF {
public:
	Mutex();
	~Mutex();
	ReturnValue_t lockMutex(TimeoutType timeoutType, uint32_t timeoutMs = 0);
	ReturnValue_t unlockMutex();
private:
	rtems_id mutexId;
	static uint8_t count;
};

#endif /* OS_RTEMS_MUTEX_H_ */

#ifndef OS_RTEMS_MUTEX_H_
#define OS_RTEMS_MUTEX_H_

#include <framework/ipc/MutexIF.h>


#include <FreeRTOS.h>
#include "semphr.h"

/**
 *
 * @ingroup osal
 */
class Mutex : public MutexIF {
public:
	Mutex();
	~Mutex();
	ReturnValue_t lockMutex(uint32_t timeoutMs);
	ReturnValue_t unlockMutex();
private:
	SemaphoreHandle_t handle;
};

#endif /* OS_RTEMS_MUTEX_H_ */

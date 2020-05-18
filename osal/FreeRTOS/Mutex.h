#ifndef OS_RTEMS_MUTEX_H_
#define OS_RTEMS_MUTEX_H_

#include <framework/ipc/MutexIF.h>

extern "C" {
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
}


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

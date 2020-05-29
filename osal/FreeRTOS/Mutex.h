#ifndef FRAMEWORK_FREERTOS_MUTEX_H_
#define FRAMEWORK_FREERTOS_MUTEX_H_

#include <framework/ipc/MutexIF.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

/**
 * @brief OS component to implement MUTual EXclusion
 *
 * @details
 * Mutexes are binary semaphores which include a priority inheritance mechanism.
 * Documentation: https://www.freertos.org/Real-time-embedded-RTOS-mutexes.html
 * @ingroup osal
 */
class Mutex : public MutexIF {
public:
	Mutex();
	~Mutex();
	ReturnValue_t lockMutex(uint32_t timeoutMs = MutexIF::MAX_TIMEOUT) override;
	ReturnValue_t unlockMutex() override;
private:
	SemaphoreHandle_t handle;
};

#endif /* FRAMEWORK_FREERTOS_MUTEX_H_ */

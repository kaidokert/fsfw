#ifndef FRAMEWORK_OSAL_FREERTOS_MUTEX_H_
#define FRAMEWORK_OSAL_FREERTOS_MUTEX_H_

#include <framework/ipc/MutexIF.h>

#include <mutex>

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
	Mutex() = default;
	ReturnValue_t lockMutex(uint32_t timeoutMs = MutexIF::BLOCKING) override;
	ReturnValue_t unlockMutex() override;

	std::timed_mutex* getMutexHandle();
private:
	bool locked = false;
	std::timed_mutex mutex;
};

#endif /* FRAMEWORK_FREERTOS_MUTEX_H_ */

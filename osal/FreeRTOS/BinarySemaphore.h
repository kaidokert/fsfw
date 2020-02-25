/**
 * @file BinarySempahore.h
 *
 * @date 25.02.2020
 */
#ifndef FRAMEWORK_OSAL_FREERTOS_BINARYSEMPAHORE_H_
#define FRAMEWORK_OSAL_FREERTOS_BINARYSEMPAHORE_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <FreeRTOS.h>
#include "semphr.h"

/**
 * @brief OS Tool to achieve synchronization of between tasks or between task and ISR
 * @details
 * Documentation: https://www.freertos.org/Embedded-RTOS-Binary-Semaphores.html
 * @ingroup osal
 */
class BinarySemaphore: public HasReturnvaluesIF {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::SEMAPHORE_IF;

	/** Semaphore object not found */
	static const ReturnValue_t SEMAPHORE_NOT_FOUND = MAKE_RETURN_CODE(1);
	/** Semaphore timeout */
	static const ReturnValue_t SEMAPHORE_TIMEOUT = MAKE_RETURN_CODE(2);
	/** The current semaphore can not be given, because it is not owned */
	static const ReturnValue_t SEMAPHORE_NOT_OWNED = MAKE_RETURN_CODE(3);

	/**
	 * Create a binary semaphore
	 */
	BinarySemaphore();

	/**
	 * Delete the binary semaphore to prevent a memory leak
	 */
	~BinarySemaphore();

	/**
	 * Take the binary semaphore.
	 * If the semaphore has already been taken, the task will be blocked for a maximum
	 * of #timeoutMs or until the semaphore is given back,
	 * for example by an ISR or another task.
	 * @param timeoutMs
	 * @return -@c RETURN_OK on success
	 *         -@c RETURN_FAILED on failure
	 */
	ReturnValue_t takeBinarySemaphore(uint32_t timeoutMs);

	/**
	 * Same as lockBinarySemaphore() with timeout in FreeRTOS ticks.
	 * @param timeoutTicks
	 * @return -@c RETURN_OK on success
	 *         -@c RETURN_FAILED on failure
	 */
	ReturnValue_t takeBinarySemaphoreTickTimeout(TickType_t timeoutTicks);

	/**
	 * Give back the binary semaphore
	 * @return -@c RETURN_OK on success
	 *         -@c RETURN_FAILED on failure
	 */
	ReturnValue_t giveBinarySemaphore();

	/**
	 * Get Handle to the semaphore.
	 * @return
	 */
	SemaphoreHandle_t getSemaphore();
private:
	SemaphoreHandle_t handle;
};

/**
 * Wrapper function to give back semaphore from handle
 * @param semaphore
 * @return -@c RETURN_OK on success
 *         -@c RETURN_FAILED on failure
 */
ReturnValue_t giveBinarySemaphore(SemaphoreHandle_t semaphore);

/**
 * Wrapper function to give back semaphore from handle when called from an ISR
 * @param semaphore
 * @param higherPriorityTaskWoken This will be set to pdPASS if a task with a higher priority
 *        was unblocked
 * @return -@c RETURN_OK on success
 *         -@c RETURN_FAILED on failure
 */
ReturnValue_t giveBinarySemaphoreFromISR(SemaphoreHandle_t semaphore,
			BaseType_t * higherPriorityTaskWoken);

#endif /* FRAMEWORK_OSAL_FREERTOS_BINARYSEMPAHORE_H_ */

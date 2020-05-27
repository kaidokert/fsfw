#ifndef FRAMEWORK_OSAL_FREERTOS_BINARYSEMPAHORE_H_
#define FRAMEWORK_OSAL_FREERTOS_BINARYSEMPAHORE_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/tasks/SemaphoreIF.h>

extern "C" {
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
}

/**
 * @brief 	OS Tool to achieve synchronization of between tasks or between
 * 			task and ISR. The default semaphore implementation creates a
 * 			binary semaphore, which can only be taken once.
 * @details
 * Documentation: https://www.freertos.org/Embedded-RTOS-Binary-Semaphores.html
 *
 * Please note that if the semaphore implementation is only related to
 * the synchronization of one task, the new task notifications can be used,
 * also see the BinSemaphUsingTask and CountingSemaphUsingTask classes.
 * These use the task notification value instead of a queue and are
 * faster and more efficient.
 *
 * @author 	R. Mueller
 * @ingroup osal
 */
class BinarySemaphore: public SemaphoreIF,
		public HasReturnvaluesIF {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::SEMAPHORE_IF;

	//! @brief Default ctor
	BinarySemaphore();
	//! @brief Copy ctor, deleted explicitely.
	BinarySemaphore(const BinarySemaphore&) = delete;
	//! @brief Copy assignment, deleted explicitely.
	BinarySemaphore& operator=(const BinarySemaphore&) = delete;
	//! @brief Move ctor
	BinarySemaphore (BinarySemaphore &&);
	//! @brief Move assignment
	BinarySemaphore & operator=(BinarySemaphore &&);
	//! @brief Destructor
	virtual ~BinarySemaphore();

	ReturnValue_t acquire(uint32_t timeoutMs =
	        SemaphoreIF::NO_TIMEOUT) override;
	ReturnValue_t release() override;
	uint8_t getSemaphoreCounter() override;

	/**
	 * Take the binary semaphore.
	 * If the semaphore has already been taken, the task will be blocked
	 * for a maximum of #timeoutMs or until the semaphore is given back,
	 * for example by an ISR or another task.
	 * @param timeoutMs
	 * @return -@c RETURN_OK on success
	 *         -@c RETURN_FAILED on failure
	 */
	ReturnValue_t takeBinarySemaphore(uint32_t timeoutMs =
	       	   SemaphoreIF::NO_TIMEOUT);

	/**
	 * Same as lockBinarySemaphore() with timeout in FreeRTOS ticks.
	 * @param timeoutTicks
	 * @return - @c RETURN_OK on success
	 *         - @c RETURN_FAILED on failure
	 */
	ReturnValue_t takeBinarySemaphoreTickTimeout(TickType_t timeoutTicks =
	        BinarySemaphore::NO_TIMEOUT);

	/**
	 * Give back the binary semaphore
	 * @return - @c RETURN_OK on success
	 *         - @c RETURN_FAILED on failure
	 */
	ReturnValue_t giveBinarySemaphore();

	/**
	 * Get Handle to the semaphore.
	 * @return
	 */
	SemaphoreHandle_t getSemaphore();

	 /**
	 * Wrapper function to give back semaphore from handle
	 * @param semaphore
	 * @return - @c RETURN_OK on success
	 *         - @c RETURN_FAILED on failure
	 */
	static ReturnValue_t giveBinarySemaphore(SemaphoreHandle_t semaphore);

	/**
	 * Wrapper function to give back semaphore from handle when called from an ISR
	 * @param semaphore
	 * @param higherPriorityTaskWoken This will be set to pdPASS if a task with a higher priority
	 *        was unblocked
	 * @return - @c RETURN_OK on success
	 *         - @c RETURN_FAILED on failure
	 */
	static ReturnValue_t giveBinarySemaphoreFromISR(SemaphoreHandle_t semaphore,
				BaseType_t * higherPriorityTaskWoken);

protected:
	SemaphoreHandle_t handle;
};

#endif /* FRAMEWORK_OSAL_FREERTOS_BINARYSEMPAHORE_H_ */

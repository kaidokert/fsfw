#ifndef FRAMEWORK_OSAL_FREERTOS_BINARYSEMPAHORE_H_
#define FRAMEWORK_OSAL_FREERTOS_BINARYSEMPAHORE_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/tasks/SemaphoreIF.h>
extern "C" {
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
}

// TODO: Counting semaphores and implement the new (better)
//		 task notifications. However, those use task notifications require
//		 the task handle. Maybe it would be better to make a separate class
// 		 and switch between the classes with #ifdefs.
//		 Task Notifications require FreeRTOS V8.2 something..
/**
 * @brief 	OS Tool to achieve synchronization of between tasks or between
 * 			task and ISR. The default semaphore implementation creates a
 * 			binary semaphore, which can only be taken once.
 * @details
 * Documentation: https://www.freertos.org/Embedded-RTOS-Binary-Semaphores.html
 *
 * @author 	R. Mueller
 * @ingroup osal
 */
class BinarySemaphore: public SemaphoreIF,
		public HasReturnvaluesIF {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::SEMAPHORE_IF;

	//! No block time, poll the semaphore. Can also be used as tick type.
	//! Can be passed as tick type and ms value.
	static constexpr uint32_t NO_BLOCK_TIMEOUT = 0;
	static constexpr TickType_t NO_BLOCK_TICKS = 0;
	//! No block time, poll the semaphore.
	//! Can be passed as tick type and ms value.
	static constexpr TickType_t BLOCK_TIMEOUT_TICKS = portMAX_DELAY;
	static constexpr uint32_t BLOCK_TIMEOUT = portMAX_DELAY;

	//! @brief Default ctor
	BinarySemaphore();
	//! @brief Copy ctor
	BinarySemaphore(const BinarySemaphore&);
	//! @brief Copy assignment
	BinarySemaphore& operator=(const BinarySemaphore&);
	//! @brief Move ctor
	BinarySemaphore (BinarySemaphore &&);
	//! @brief Move assignment
	BinarySemaphore & operator=(BinarySemaphore &&);
	//! @brief Destructor
	virtual ~BinarySemaphore();

	ReturnValue_t acquire(uint32_t timeoutMs =
	        BinarySemaphore::NO_BLOCK_TIMEOUT) override;
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
	        BinarySemaphore::NO_BLOCK_TIMEOUT);

	/**
	 * Same as lockBinarySemaphore() with timeout in FreeRTOS ticks.
	 * @param timeoutTicks
	 * @return - @c RETURN_OK on success
	 *         - @c RETURN_FAILED on failure
	 */
	ReturnValue_t takeBinarySemaphoreTickTimeout(TickType_t timeoutTicks =
	        BinarySemaphore::NO_BLOCK_TICKS);

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
	 * Reset the semaphore.
	 */
	void resetSemaphore();

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

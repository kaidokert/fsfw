#ifndef FRAMEWORK_OSAL_FREERTOS_BINARYSEMPAHORE_H_
#define FRAMEWORK_OSAL_FREERTOS_BINARYSEMPAHORE_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/tasks/SemaphoreIF.h>

extern "C" {
#include <semaphore.h>
}

/**
 * @brief 	OS Tool to achieve synchronization of between tasks or between
 * 			task and ISR. The default semaphore implementation creates a
 * 			binary semaphore, which can only be taken once.
 * @details
 * See: http://www.man7.org/linux/man-pages/man7/sem_overview.7.html
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

	void initSemaphore();

	uint8_t getSemaphoreCounter() const override;
	static uint8_t getSemaphoreCounter(sem_t* handle);
	/**
	 * Take the binary semaphore.
	 * If the semaphore has already been taken, the task will be blocked
	 * for a maximum of #timeoutMs or until the semaphore is given back,
	 * for example by an ISR or another task.
	 * @param timeoutMs
	 * @return -@c RETURN_OK on success
	 *         -@c SemaphoreIF::SEMAPHORE_TIMEOUT on timeout
	 */
	ReturnValue_t acquire(uint32_t timeoutMs =
	       	   SemaphoreIF::NO_TIMEOUT) override;

	/**
	 * Release the binary semaphore.
	 * @return -@c RETURN_OK on success
	 *         -@c SemaphoreIF::SEMAPHORE_NOT_OWNED if the semaphores is
	 *         	already available.
	 */
	ReturnValue_t release() override;

	static ReturnValue_t release(sem_t* handle);

protected:
	sem_t handle;
};

#endif /* FRAMEWORK_OSAL_FREERTOS_BINARYSEMPAHORE_H_ */

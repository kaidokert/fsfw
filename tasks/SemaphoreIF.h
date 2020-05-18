#ifndef FRAMEWORK_TASKS_SEMAPHOREIF_H_
#define FRAMEWORK_TASKS_SEMAPHOREIF_H_
#include <framework/returnvalues/FwClassIds.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <cstdint>

/**
 * @brief 	Generic interface for semaphores, which can be used to achieve
 * 			task synchronization.
 */
class SemaphoreIF {
public:
	virtual~ SemaphoreIF() {};
	//!< Needs to be defined in implementation.
	static const uint32_t NO_TIMEOUT;
	static const uint8_t INTERFACE_ID = CLASS_ID::SEMAPHORE_IF;
	//! Semaphore timeout
	static constexpr ReturnValue_t SEMAPHORE_TIMEOUT = MAKE_RETURN_CODE(1);
	//! The current semaphore can not be given, because it is not owned
	static constexpr ReturnValue_t SEMAPHORE_NOT_OWNED = MAKE_RETURN_CODE(2);
	static constexpr ReturnValue_t SEMAPHORE_NULLPOINTER = MAKE_RETURN_CODE(3);

	/**
	 * Generic call to acquire a semaphore.
	 * If there are no more semaphores to be taken (for a counting semaphore,
	 * a semaphore may be taken more than once), the taks will block
	 * for a maximum of timeoutMs while trying to acquire the semaphore.
	 * This can be used to achieve task synchrnization.
	 * @param timeoutMs
	 * @return - c RETURN_OK for successfull acquisition
	 */
	virtual ReturnValue_t acquire(uint32_t timeoutMs) = 0;

	/**
	 * Corrensponding call to release a semaphore.
	 * @return -@c RETURN_OK for successfull release
	 */
	virtual ReturnValue_t release() = 0;

	/**
	 * If the semaphore is a counting semaphore then the semaphores current
	 * count value is returned. If the semaphore is a binary semaphore then 1
	 * is returned if the semaphore is available, and 0 is returned if the
	 * semaphore is not available.
	 */
	virtual uint8_t getSemaphoreCounter() = 0;
};

#endif /* FRAMEWORK_TASKS_SEMAPHOREIF_H_ */

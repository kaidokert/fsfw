#ifndef FRAMEWORK_IPC_MUTEXIF_H_
#define FRAMEWORK_IPC_MUTEXIF_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>

/**
 * @brief Common interface for OS Mutex objects which provide MUTual EXclusion.
 *
 * @details https://en.wikipedia.org/wiki/Lock_(computer_science)
 * @ingroup osal
 * @ingroup interface
 */
class MutexIF {
public:
	static const uint32_t NO_TIMEOUT; //!< Needs to be defined in implementation.

	static const uint8_t INTERFACE_ID = CLASS_ID::MUTEX_IF;
	/**
	 * The system lacked the necessary resources (other than memory) to initialize another mutex.
	 */
	static const ReturnValue_t NOT_ENOUGH_RESOURCES = MAKE_RETURN_CODE(1);
	/**
	 * Insufficient memory to create or init Mutex
	 */
	static const ReturnValue_t INSUFFICIENT_MEMORY = MAKE_RETURN_CODE(2);
	/**
	 * Caller does not have enough or right privilege
	 */
	static const ReturnValue_t NO_PRIVILEGE =  MAKE_RETURN_CODE(3);
	/**
	 * Wrong Attribute Setting
	 */
	static const ReturnValue_t WRONG_ATTRIBUTE_SETTING = MAKE_RETURN_CODE(4);
	/**
	 * The mutex is already locked
	 */
	static const ReturnValue_t MUTEX_ALREADY_LOCKED = MAKE_RETURN_CODE(5);
	/**
	 * Mutex object not found
	 */
	static const ReturnValue_t MUTEX_NOT_FOUND = MAKE_RETURN_CODE(6);
	/**
	 * Mutex could not be locked because max amount of recursive locks
	 */
	static const ReturnValue_t MUTEX_MAX_LOCKS = MAKE_RETURN_CODE(7);
	/**
	 * The current thread already owns this mutex
	 */
	static const ReturnValue_t CURR_THREAD_ALREADY_OWNS_MUTEX = MAKE_RETURN_CODE(8);
	/**
	 * Current thread does not own this mutex
	 */
	static const ReturnValue_t CURR_THREAD_DOES_NOT_OWN_MUTEX = MAKE_RETURN_CODE(9);
	/**
	 * The Mutex could not be blocked before timeout
	 */
	static const ReturnValue_t MUTEX_TIMEOUT = MAKE_RETURN_CODE(10);
	/**
	 * Invalid Mutex ID
	 */
	static const ReturnValue_t MUTEX_INVALID_ID = MAKE_RETURN_CODE(11);
	/**
	 * Mutex destroyed while waiting
	 */
	static const ReturnValue_t MUTEX_DESTROYED_WHILE_WAITING = MAKE_RETURN_CODE(12);

	virtual ~MutexIF() {}
	virtual ReturnValue_t lockMutex(uint32_t timeoutMs) = 0;
	virtual ReturnValue_t unlockMutex() = 0;
};



#endif /* FRAMEWORK_IPC_MUTEXIF_H_ */

#ifndef FRAMEWORK_TASK_PERIODICTASKIF_H_
#define FRAMEWORK_TASK_PERIODICTASKIF_H_

#include "../objectmanager/SystemObjectIF.h"
#include "../timemanager/Clock.h"
#include <cstddef>
class ExecutableObjectIF;

/**
 * New version of TaskIF
 * Follows RAII principles, i.e. there's no create or delete method.
 * Minimalistic.
*/
class PeriodicTaskIF {
public:
	static const size_t MINIMUM_STACK_SIZE;
	/**
	 * @brief	A virtual destructor as it is mandatory for interfaces.
	 */
	virtual ~PeriodicTaskIF() { }
	/**
	 * @brief	With the startTask method, a created task can be started
	 *          for the first time.
	 */
	virtual ReturnValue_t startTask() = 0;

	/**
	 * Add a component (object) to a periodic task. The pointer to the
	 * task can be set optionally
	 * @param object
	 * Add an object to the task. The most important case is to add an
	 * executable object with a function which will be called regularly
	 * (see ExecutableObjectIF)
	 * @param setTaskIF
	 * Can be used to specify whether the task object pointer is passed
	 * to the component.
	 * @return
	 */
	virtual ReturnValue_t addComponent(object_id_t object) {
	    return HasReturnvaluesIF::RETURN_FAILED;
	};

	virtual ReturnValue_t sleepFor(uint32_t ms) = 0;

	virtual uint32_t getPeriodMs() const = 0;
};


#endif /* PERIODICTASKIF_H_ */

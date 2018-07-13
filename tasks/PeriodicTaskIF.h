#ifndef PERIODICTASKIF_H_
#define PERIODICTASKIF_H_

#include <framework/objectmanager/SystemObjectIF.h>
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
	 * @brief	With the startTask method, a created task can be started for the first time.
	 */
	virtual ReturnValue_t startTask() = 0;

	virtual ReturnValue_t addComponent(object_id_t object) {return HasReturnvaluesIF::RETURN_FAILED;};

	virtual ReturnValue_t sleepFor(uint32_t ms) = 0;

	virtual uint32_t getPeriodMs() const = 0;
};


#endif /* PERIODICTASKIF_H_ */

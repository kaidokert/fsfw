#ifndef FRAMEWORK_TASKS_TASKFACTORY_H_
#define FRAMEWORK_TASKS_TASKFACTORY_H_

#include <stdlib.h>
#include "FixedTimeslotTaskIF.h"
#include "Typedef.h"

/**
 * Singleton Class that produces Tasks.
 */
class TaskFactory {
public:
	virtual ~TaskFactory();
	/**
	 * Returns the single instance of TaskFactory.
	 * The implementation of #instance is found in its subclasses.
	 * Thus, we choose link-time variability of the  instance.
	 */
	static TaskFactory* instance();

	/**
	 * Creates a new periodic task and returns the interface pointer.
	 * @param name_ Name of the task
	 * @param taskPriority_ Priority of the task
	 * @param stackSize_ Stack Size of the task
	 * @param period_ Period of the task
	 * @param deadLineMissedFunction_ Function to be called if a deadline was missed
	 * @return PeriodicTaskIF* Pointer to the newly created Task
	 */
	PeriodicTaskIF* createPeriodicTask(TaskName name_,
			TaskPriority taskPriority_, TaskStackSize stackSize_,
			TaskPeriod periodInSeconds_,
			TaskDeadlineMissedFunction deadLineMissedFunction_);

	/**
	 *
	 * @param name_ Name of the task
	 * @param taskPriority_ Priority of the task
	 * @param stackSize_ Stack Size of the task
	 * @param period_ Period of the task
	 * @param deadLineMissedFunction_ Function to be called if a deadline was missed
	 * @return FixedTimeslotTaskIF* Pointer to the newly created Task
	 */
	FixedTimeslotTaskIF* createFixedTimeslotTask(TaskName name_,
			TaskPriority taskPriority_, TaskStackSize stackSize_,
			TaskPeriod periodInSeconds_,
			TaskDeadlineMissedFunction deadLineMissedFunction_);

	/**
	 * Function to be called to delete a task
	 * @param task The pointer to the task that shall be deleted, NULL specifies current Task
	 * @return Success of deletion
	 */
	static ReturnValue_t deleteTask(PeriodicTaskIF* task = NULL);

	/**
	 * Function to be called to delay current task
	 * @param delay The delay in milliseconds
	 * @return Success of deletion
	 */
	static ReturnValue_t delayTask(uint32_t delayMs);

private:
	/**
	 * External instantiation is not allowed.
	 */
	TaskFactory();
	static TaskFactory* factoryInstance;

};

#endif /* FRAMEWORK_TASKS_TASKFACTORY_H_ */

#ifndef FSFW_TASKS_TASKFACTORY_H_
#define FSFW_TASKS_TASKFACTORY_H_

#include "FixedTimeslotTaskIF.h"
#include "Typedef.h"

#include <cstdlib>

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
	 * @param task The pointer to the task that shall be deleted,
	 * nullptr specifies current Task
	 * @return Success of deletion
	 */
	static ReturnValue_t deleteTask(PeriodicTaskIF* task = nullptr);

	/**
	 * Function to be called to delay current task
	 * @param delay The delay in milliseconds
	 * @return Success of deletion
	 */
	static ReturnValue_t delayTask(uint32_t delayMs);

	/**
	 * OS specific implementation to print deadline. In most cases, there is a OS specific
	 * way to retrieve the task name and print it out as well.
	 */
	static void printMissedDeadline();

private:
	/**
	 * External instantiation is not allowed.
	 */
	TaskFactory();
	static TaskFactory* factoryInstance;

};

#endif /* FSFW_TASKS_TASKFACTORY_H_ */

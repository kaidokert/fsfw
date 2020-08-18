#ifndef FRAMEWORK_TASKS_TASKFACTORY_H_
#define FRAMEWORK_TASKS_TASKFACTORY_H_

#include <cstdlib>
#include "../tasks/FixedTimeslotTaskIF.h"
#include "../tasks/Typedef.h"

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
	 * Generic interface to create a periodic task
	 * @param name_ Name of the task
	 * @param taskPriority_ Priority of the task
	 * @param stackSize_ Stack size if the task
	 * @param periodInSeconds_ Period in seconds
	 * @param deadLineMissedFunction_ This function is called if a deadline was
	 * missed
	 * @return Pointer to the created periodic task class
	 */
	PeriodicTaskIF* createPeriodicTask(TaskName name_,
			TaskPriority taskPriority_, TaskStackSize stackSize_,
			TaskPeriod periodInSeconds_,
			TaskDeadlineMissedFunction deadLineMissedFunction_);

	/**
	 * Generic interface to create a fixed timeslot task
	 * @param name_ Name of the task
	 * @param taskPriority_ Priority of the task
	 * @param stackSize_ Stack size if the task
	 * @param periodInSeconds_ Period in seconds
	 * @param deadLineMissedFunction_ This function is called if a deadline was
	 * missed
	 * @return Pointer to the created periodic task class
	 */
	FixedTimeslotTaskIF* createFixedTimeslotTask(TaskName name_,
			TaskPriority taskPriority_, TaskStackSize stackSize_,
			TaskPeriod periodInSeconds_,
			TaskDeadlineMissedFunction deadLineMissedFunction_);

	/**
	 * Function to be called to delete a task
	 * @param task The pointer to the task that shall be deleted,
	 * 				NULL specifies current Task
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

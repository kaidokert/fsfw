#ifndef FRAMEWORK_TASKS_TASKFACTORY_H_
#define FRAMEWORK_TASKS_TASKFACTORY_H_

#include <framework/tasks/FixedTimeslotTaskIF.h>
#include <framework/tasks/Typedef.h>




/**
 * Singleton Class that produces Tasks.
 */
class TaskFactory{
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
	PeriodicTaskIF* createPeriodicTask(OSAL::TaskName name_,OSAL::TaskPriority taskPriority_,OSAL::TaskStackSize stackSize_,OSAL::TaskPeriod periodInSeconds_,OSAL::TaskDeadlineMissedFunction deadLineMissedFunction_);

	/**
	 *
	 * @param name_ Name of the task
	 * @param taskPriority_ Priority of the task
	 * @param stackSize_ Stack Size of the task
	 * @param period_ Period of the task
	 * @param deadLineMissedFunction_ Function to be called if a deadline was missed
	 * @return FixedTimeslotTaskIF* Pointer to the newly created Task
	 */
	FixedTimeslotTaskIF* createFixedTimeslotTask(OSAL::TaskName name_,OSAL::TaskPriority taskPriority_,OSAL::TaskStackSize stackSize_,OSAL::TaskPeriod periodInSeconds_,OSAL::TaskDeadlineMissedFunction deadLineMissedFunction_);


	/**
	 * Function to be called to delete a task
	 * @param task The pointer to the task that shall be deleted
	 * @return Success of deletion
	 */
	ReturnValue_t deleteTask(PeriodicTaskIF* task);

private:
	/**
	 * External instantiation is not allowed.
	 */
	TaskFactory();
	static TaskFactory* factoryInstance;


};



#endif /* FRAMEWORK_TASKS_TASKFACTORY_H_ */

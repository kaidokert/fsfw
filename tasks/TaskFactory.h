#ifndef FRAMEWORK_TASKS_TASKFACTORY_H_
#define FRAMEWORK_TASKS_TASKFACTORY_H_

#include <cstdlib>
#include <framework/tasks/FixedTimeslotTaskIF.h>
#include <framework/tasks/Typedef.h>

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
	 * Keep in Mind that you need to call before this vTaskStartScheduler()!
	 * A lot of task parameters are set in "FreeRTOSConfig.h".
	 * @param name_ Name of the task, lenght limited by configMAX_TASK_NAME_LEN
	 * @param taskPriority_ Number of priorities specified by
	 * configMAX_PRIORITIES. High taskPriority_ number means high priority.
	 * @param stackSize_ 	Stack size in words (not bytes!).
	 * Lower limit specified by configMINIMAL_STACK_SIZE
	 * @param period_		Period in seconds.
	 * @param deadLineMissedFunction_ Callback if a deadline was missed.
	 * @return Pointer to the newly created task.
	 */
	PeriodicTaskIF* createPeriodicTask(TaskName name_,
			TaskPriority taskPriority_, TaskStackSize stackSize_,
			TaskPeriod periodInSeconds_,
			TaskDeadlineMissedFunction deadLineMissedFunction_);

	/**
	 * Keep in Mind that you need to call before this vTaskStartScheduler()!
	 * A lot of task parameters are set in "FreeRTOSConfig.h".
	 * @param name_ Name of the task, lenght limited by configMAX_TASK_NAME_LEN
	 * @param taskPriority_ Number of priorities specified by
	 * configMAX_PRIORITIES. High taskPriority_ number means high priority.
	 * @param stackSize_ 	Stack size in words (not bytes!).
	 * Lower limit specified by configMINIMAL_STACK_SIZE
	 * @param period_		Period in seconds.
	 * @param deadLineMissedFunction_ Callback if a deadline was missed.
	 * @return Pointer to the newly created task.
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

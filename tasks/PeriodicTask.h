/**
 * @file PeriodicTask.h
 *
 * @brief This file contains the definition for the PeriodicTask class.
 *
 * @author Bastian Baetz
 *
 * @date 07/21/2010
 *
 */
#ifndef OPUSPERIODICTASK_H_
#define OPUSPERIODICTASK_H_

#include <framework/tasks/TaskBase.h>

/**
 *
 * @brief This class represents a specialized task for periodic activities.
 *
 * @details A simple, but very important task type is the periodic task. Each task of this type has
 * 			a certain period assigned. When started, the task's functionality (a simple function)
 * 			is executed. On finishing, the task is blocked for the rest of the period and restarted
 * 			afterwards. A missed deadline is detected and a function to perform necessary failure
 * 			detection may be called.
 *
 * @author Bastian Baetz
 *
 * @date 07/21/2010
 *
 * @ingroup task_handling
 */
class PeriodicTask: public TaskBase {
protected:

	/**
	 * @brief	The period of the task.
	 * @details	The period determines the frequency of the task's execution. It is expressed in clock ticks.
	 */
	Interval_t period;

	/**
	* @brief id of the associated OS period
	*/
	PeriodId_t periodId;

	/**
	 * @brief	This is the function executed in the new task's context.
	 * @details	It converts the argument back to the thread object type and copies the class instance
	 * 			to the task context. The taskFunctionality method is called afterwards.
	 * @param	A pointer to the task object itself is passed as argument.
	 */
	static TaskReturn_t taskEntryPoint( TaskArgument_t argument );

	/**
	 * @brief	The pointer to the deadline-missed function.
	 * @details	This pointer stores the function that is executed if the task's deadline is missed.
	 * 			So, each may react individually on a timing failure. The pointer may be NULL,
	 * 			then nothing happens on missing the deadline. The deadline is equal to the next execution
	 * 			of the periodic task.
	 */
	void ( *deadlineMissedFunc )( void );

	/**
	 * @brief	The function containing the actual functionality of the task.
	 * @image latex act_OPUSPeriodicThread.eps "Activity diagram of the PeriodicThread functionality." width=0.6@textwidth
	 * @image html 	act_OPUSPeriodicThread.png "Activity diagram of the PeriodicThread functionality."
	 * @details	The figure above shows the functional execution of this method. It sets and starts
	 * 			the task's period, then enters a loop that is repeated as long as the isRunning
	 * 			attribute is true. Within the loop, the taskFunction is called, and
	 * 			afterwards the checkAndRestartPeriod system call to block the task until the next
	 * 			period. On missing the deadline, the deadlineMissedFunction is executed.
	 */
	void taskFunctionality( void );
	/**
	 * @brief	In this attribute the pointer to the function which shall be executed periodically
	 * 			is stored.
	 */
	ReturnValue_t ( *taskFunction )(  TaskBase* );
public:
	/**
	 * @brief	The standard constructor of the class.
	 * @details	This is the general constructor of the class. In the underlying TaskBase class,
	 * 			a new operating system task is created. In addition to the TaskBase parameters,
	 * 			the period, the actual function to be executed and an optional "deadline-missed"
	 * 			function pointer is passed.
	 * @param priority		Sets the priority of a task. Values range from a low 0 to a high 99.
	 * @param stack_size	The stack size reserved by the operating system for the task.
	 * @param setPeriod 	The length of the period with which the task's functionality will be
	 * 						executed. It is expressed in clock ticks.
	 * @param (*setDeadlineMissedFunc)()	The function pointer to the deadline missed function
	 * 										that shall be assigned.
	 * @param ( *setTaskFunction )(  TaskBase* )	A pointer to the actual function to be executed.
	 */
	PeriodicTask(  const char *name, TaskPriority_t setPriority, size_t setStack, Interval_t setPeriod, void ( *setDeadlineMissedFunc )(), ReturnValue_t ( *setTaskFunction )(  TaskBase* ) );

	/**
	 * @brief	The destructor of the class.
	 * @details	Similar to the destructor in the parent class, no memory clean-ups are necessary.
	 * 			Thus, the destructor is empty.
	 */
	virtual ~PeriodicTask( void );

	/**
	 * @brief	The method to start the task.
	 * @details	The method starts the task with the respective system call.
	 * 			Entry point is the taskEntryPoint method described below.
	 * 			The address of the task object is passed as an argument
	 * 			to the system call.
	 */
	virtual ReturnValue_t startTask( void );
};

#endif /* OPUSPERIODICTASK_H_ */

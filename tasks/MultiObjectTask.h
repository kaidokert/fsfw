/**
 * @file	MultiObjectTask.h
 * @brief	This file defines the MultiObjectTask class.
 * @date	30.01.2014
 * @author	baetz
 */
#ifndef MULTIOBJECTTASK_H_
#define MULTIOBJECTTASK_H_

#include <framework/objectmanager/ObjectManagerIF.h>
#include <framework/tasks/ExecutableObjectIF.h>
#include <framework/tasks/TaskBase.h>
#include <list>
/**
 * @brief This class represents a specialized task for periodic activities of multiple objects.
 *
 * @details MultiObjectTask is an extension to ObjectTask in the way that it is able to execute
 * 			multiple objects that implement the ExecutableObjectIF interface. The objects must be
 * 			added prior to starting the task.
 *
 * @ingroup task_handling
 */
class MultiObjectTask: public TaskBase {
protected:
	typedef std::list<ExecutableObjectIF*> ObjectList;	//!< Typedef for the List of objects.
	/**
	 * @brief	This attribute holds a list of objects to be executed.
	 */
	ObjectList objectList;
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
	 * @brief	The pointer to the deadline-missed function.
	 * @details	This pointer stores the function that is executed if the task's deadline is missed.
	 * 			So, each may react individually on a timing failure. The pointer may be NULL,
	 * 			then nothing happens on missing the deadline. The deadline is equal to the next execution
	 * 			of the periodic task.
	 */
	void (*deadlineMissedFunc)(void);
	/**
	 * @brief	This is the function executed in the new task's context.
	 * @details	It converts the argument back to the thread object type and copies the class instance
	 * 			to the task context. The taskFunctionality method is called afterwards.
	 * @param	A pointer to the task object itself is passed as argument.
	 */
	static TaskReturn_t taskEntryPoint(TaskArgument_t argument);
	/**
	 * @brief	The function containing the actual functionality of the task.
	 * @details	The method sets and starts
	 * 			the task's period, then enters a loop that is repeated as long as the isRunning
	 * 			attribute is true. Within the loop, all performOperation methods of the added
	 * 			objects are called. Afterwards the checkAndRestartPeriod system call blocks the task
	 * 			until the next period.
	 * 			On missing the deadline, the deadlineMissedFunction is executed.
	 */
	void taskFunctionality(void);

public:
	/**
	 * @brief	Standard constructor of the class.
	 * @details	The class is initialized without allocated objects. These need to be added
	 * 			with #addObject.
	 * 			In the underlying TaskBase class, a new operating system task is created.
	 * 			In addition to the TaskBase parameters, the period, the pointer to the
	 * 			aforementioned initialization function and an optional "deadline-missed"
	 * 			function pointer is passed.
	 * @param priority		Sets the priority of a task. Values range from a low 0 to a high 99.
	 * @param stack_size	The stack size reserved by the operating system for the task.
	 * @param setPeriod 	The length of the period with which the task's functionality will be
	 * 						executed. It is expressed in clock ticks.
	 * @param setDeadlineMissedFunc	The function pointer to the deadline missed function
	 * 								that shall be assigned.
	 */
	MultiObjectTask(const char *name, TaskPriority_t setPriority, size_t setStack, Interval_t setPeriod,
			void (*setDeadlineMissedFunc)());
	/**
	 * @brief	Currently, the executed object's lifetime is not coupled with the task object's
	 * 			lifetime, so the destructor is empty.
	 */
	virtual ~MultiObjectTask(void);

	/**
	 * @brief	The method to start the task.
	 * @details	The method starts the task with the respective system call.
	 * 			Entry point is the taskEntryPoint method described below.
	 * 			The address of the task object is passed as an argument
	 * 			to the system call.
	 */
	ReturnValue_t startTask(void);
	/**
	 * Adds an object to the list of objects to be executed.
	 * The objects are executed in the order added.
	 * @param object Id of the object to add.
	 * @return RETURN_OK on success, RETURN_FAILED if the object could not be added.
	 */
	ReturnValue_t addObject(object_id_t object);
	/**
	 * Variant to add objects with known location directly.
	 * @param object	Reference to a persistant executable object.
	 * @return RETURN_OK on success, RETURN_FAILED if the object could not be added.
	 */
	ReturnValue_t addObject(ExecutableObjectIF* object);

};

#endif /* MULTIOBJECTTASK_H_ */

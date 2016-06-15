/**
 *	@file	TaskBase.h
 *
 *  @date	11/05/2012
 *	@author	Bastian Baetz
 *
 *	@brief	This file contains the definition of the TaskBase class.
 *			It is a reviewed and updated version of a file originally created
 *			by Claas Ziemke in 2010.
 */

#ifndef TASKBASE_H_
#define TASKBASE_H_



#include <framework/osal/OSAL.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/tasks/TaskIF.h>
/**
 * @brief	This is the basic task handling class.
 *
 * @details The virtual parent class contains attributes and methods to perform basic task operations.
 * 			The task is created in the constructor. Next to the main methods to set the functionality
 * 			and start the task, it also contains methods to stop execution safely and to return its
 * 			identifier.
 * 			The whole class was undergoing a major redesign by Bastian Baetz in November 2012 where
 * 			unnecessary attributes were removed and task creation was simplified. Also, the class
 * 			implements the TaskIF now.
 *
 * @author Claas Ziemke
 *
 * @date 07/23/2010
 *
 * @ingroup task_handling
 */
class TaskBase : public TaskIF, public HasReturnvaluesIF {
protected:
	/**
	 * @brief	The  task's name -a user specific information for the operating system-  is
	 * 			generated automatically with the help of this static counter.
	 */
	static uint8_t taskCounter;
	/**
	 * @brief	The class stores the task id it got assigned from the operating system in this attribute.
	 * 			If initialization fails, the id is set to zero.
	 */
	TaskId_t id;
	/**
	 * @brief	The isRunning information can be used by child classes to change its operational behavior.
	 * @details	It is not used in the TaskBase class itself, but for example in periodic tasks to leave
	 * 			the periodic activity.
	 */
	bool isRunning;
public:
	/**
	 * @brief	The constructor creates and initializes a task.
	 * @details	This is accomplished by using the operating system call to create a task. The name is
	 * 			created automatically with the help od taskCounter. Priority and stack size are
	 * 			adjustable, all other attributes are set with default values.
	 * @param priority		Sets the priority of a task. Values range from a low 0 to a high 99.
	 * @param stack_size	The stack size reserved by the operating system for the task.
	 * @param nam			The name of the Task, as a null-terminated String. Currently max 4 chars supported (excluding Null-terminator), rest will be truncated
	 */
	TaskBase( TaskPriority_t priority, size_t stack_size, const char  *name);
	/**
	 * @brief	In the destructor, the created task is deleted.
	 */
	virtual ~TaskBase();
	/**
	 * @brief	This abstract method must be implemented by child classes to successfully start a task.
	 */
	virtual ReturnValue_t startTask() = 0;
	/**
	 * @brief	This method returns the task id of this class.
	 */
	TaskId_t getId();
	/**
	 * @brief	With this method, the task "running" state can be set.
	 * @details	This typically involves leaving any kind of periodic activity.
	 */
	void setRunning( bool set );
};


#endif /* TASKBASE_H_ */

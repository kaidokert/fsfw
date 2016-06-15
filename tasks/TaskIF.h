#ifndef TASKIF_H_
#define TASKIF_H_

/**
 * \defgroup task_handling Task Handling
 * This is the group, where all classes associated with Task Handling belong to.
 * Task handling is based on the task handling methods which the operating system
 * provides (currently RTEMS).
 */

/**
 * @brief 	This interface provides all basic methods to handle task operations.
 * @details	To uniformly operate different types of tasks, this interface provides methods to
 * 			start and stop created tasks and to return the operating system's identifier.
 *
 * @date	11/05/2012
 * @ingroup task_handling
 *	author	Bastian Baetz
 */
class TaskIF {
public:
	/**
	 * @brief	A virtual destructor as it is mandatory for interfaces.
	 */
	virtual ~TaskIF() { }
	/**
	 * @brief	With the startTask method, a created task can be started for the first time.
	 */
	virtual ReturnValue_t startTask() = 0;
	/**
	 * @brief	The getId method returns the task's operating system identifier.
	 */
	virtual TaskId_t getId() = 0;
	/**
	 * @brief	With this method, the task "running" state can be set.
	 * @details	This typically involves leaving any kind of periodic activity.
	 */
	virtual void setRunning( bool set ) = 0;
};


#endif /* TASKIF_H_ */

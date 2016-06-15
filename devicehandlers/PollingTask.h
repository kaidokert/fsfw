/**
 * @file PollingTask.h
 *
 * @brief This file contains the definition for the PollingTask class.
 *
 * @author Claas Ziemke, Bastian Baetz
 *
 * @date 17.03.2011
 *
 * Copyright 2009,2010, Claas Ziemke <claas.ziemke@gmx.net>
 * 	All rights reserved
 *
 */
#ifndef POLLINGTASK_H_
#define POLLINGTASK_H_

#include <framework/devicehandlers/PollingSequence.h>
#include <framework/tasks/TaskBase.h>

/**
 *
 * @brief This class represents a specialized thread to execute polling sequences.
 *
 * @image latex seq_PST_dynamic.eps "Sequence diagram of polling sequence operation" width=1@textwidth
 * @image html seq_PST_dynamic.png "Sequence diagram of polling sequence operation"
 *
 * @details The Polling Sequence Table is executed in a task of special type, called PollingTask.
 *			After creation the polling task initializes the PST and starts taskFunctionality.
 *			An infinite loop is entered within which the iteration through the PST is done by repetitive calls of
 *			getInterval() and pollAndAdvance().
 * @ingroup task_handling
 */
class PollingTask: public TaskBase {
protected:
	/**
	* @brief id of the associated OS period
	*/
	PeriodId_t periodId;

	/**
	 * @brief	This attribute is the pointer to the complete polling sequence table object.
	 *
	 * @details	The most important attribute of the thread object.
	 * 			It holds a pointer to the complete polling sequence table object.
	 */
	PollingSequence* pst;

	/**
	 * @brief	This attribute holds a function pointer that is executed when a deadline was missed.
	 *
	 * @details	Another function may be announced to determine the actions to perform when a deadline was missed.
	 * 			Currently, only one function for missing any deadline is allowed.
	 * 			If not used, it shall be declared NULL.
	 */
	void ( *deadlineMissedFunc )( void );
	/**
	 * @brief	This is the entry point in a new polling thread.
	 *
	 * @details	This method, that is the general entry point in the new thread, is here set to generate
	 * 			and link the Polling Sequence Table to the thread object and start taskFunctionality()
	 * 			on success. If operation of the task is ended for some reason,
	 * 			the destructor is called to free allocated memory.
	 */
	static TaskReturn_t taskEntryPoint( TaskArgument_t argument );

	/**
	 * @brief	This function holds the main functionality of the thread.
	 *
	 *
	 * @details	Holding the main functionality of the task, this method is most important.
	 * 			It links the functionalities provided by PollingSequence with the OS's System Calls
	 * 			to keep the timing of the periods.
	 */
	void taskFunctionality( void );

public:
	/**
	 * @brief	The standard constructor of the class.
	 *
	 * @details	This is the general constructor of the class. In addition to the TaskBase parameters,
	 * 			the following variables are passed:
	 *
	 * @param	(*setDeadlineMissedFunc)() The function pointer to the deadline missed function that shall be assigned.
	 *
	 * @param	getPst The object id of the completely initialized polling sequence.
	 */
	PollingTask( const char *name, TaskPriority_t setPriority, size_t setStack, void (*setDeadlineMissedFunc)(), object_id_t getPst );

	/**
	 * @brief	The destructor of the class.
	 *
	 * @details	The destructor frees all heap memory that was allocated on thread initialization for the PST and
	 * 			the device handlers. This is done by calling the PST's destructor.
	 */
	virtual ~PollingTask( void );

	/**
	 * @brief 	The function to actually start a new task.
	 *
	 * @details	As described in TaskBase this method invokes the operating systems method to start a new task.
	 * 			Entry point is taskEntryPoint().
	 */
	ReturnValue_t startTask( void );
	/**
	 * This static function can be used as #deadlineMissedFunc.
	 * It counts missedDeadlines and prints the number of missed deadlines every 10th time.
	 */
	static void missedDeadlineCounter();
	/**
	 * A helper variable to count missed deadlines.
	 */
	static uint32_t deadlineMissedCount;
};

#endif /* POLLINGTASK_H_ */

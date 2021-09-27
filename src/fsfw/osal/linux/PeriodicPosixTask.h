#ifndef FRAMEWORK_OSAL_LINUX_PERIODICPOSIXTASK_H_
#define FRAMEWORK_OSAL_LINUX_PERIODICPOSIXTASK_H_

#include "../../tasks/PeriodicTaskIF.h"
#include "../../objectmanager/ObjectManagerIF.h"
#include "PosixThread.h"
#include "../../tasks/ExecutableObjectIF.h"
#include <vector>

class PeriodicPosixTask: public PosixThread, public PeriodicTaskIF {
public:
	/**
	 * Create a generic periodic task.
	 * @param name_
	 * Name, maximum allowed size of linux is 16 chars, everything else will
	 * be truncated.
	 * @param priority_
	 * Real-time priority, ranges from 1 to 99 for Linux.
	 * See: https://man7.org/linux/man-pages/man7/sched.7.html
	 * @param stackSize_
	 * @param period_
	 * @param deadlineMissedFunc_
	 */
	PeriodicPosixTask(const char* name_, int priority_, size_t stackSize_,
			uint32_t period_, void(*deadlineMissedFunc_)());
	virtual ~PeriodicPosixTask();

	/**
	 * @brief	The method to start the task.
	 * @details	The method starts the task with the respective system call.
	 * 			Entry point is the taskEntryPoint method described below.
	 * 			The address of the task object is passed as an argument
	 * 			to the system call.
	 */
	ReturnValue_t startTask() override;
	/**
	 * Adds an object to the list of objects to be executed.
	 * The objects are executed in the order added.
	 * @param object Id of the object to add.
	 * @return RETURN_OK on success, RETURN_FAILED if the object could not be added.
	 */
	ReturnValue_t addComponent(object_id_t object) override;

	uint32_t getPeriodMs() const override;

	ReturnValue_t sleepFor(uint32_t ms) override;

private:
	typedef std::vector<ExecutableObjectIF*> ObjectList;	//!< Typedef for the List of objects.
	/**
	 * @brief	This attribute holds a list of objects to be executed.
	 */
	ObjectList objectList;

	/**
	 * @brief Flag to indicate that the task was started and is allowed to run
	 */
	bool started;


	/**
	 * @brief Period of the task in milliseconds
	 */
	uint32_t periodMs;
	/**
	 * @brief	The function containing the actual functionality of the task.
	 * @details	The method sets and starts
	 * 			the task's period, then enters a loop that is repeated indefinitely. Within the loop, all performOperation methods of the added
	 * 			objects are called. Afterwards the task will be blocked until the next period.
	 * 			On missing the deadline, the deadlineMissedFunction is executed.
	 */
	virtual void taskFunctionality(void);
	/**
	 * @brief	This is the entry point in a new thread.
	 *
	 * @details	This method, that is the entry point in the new thread and calls taskFunctionality of the child class.
	 * 			Needs a valid pointer to the derived class.
	 */
	static void* taskEntryPoint(void* arg);
	/**
	 * @brief	The pointer to the deadline-missed function.
	 * @details	This pointer stores the function that is executed if the task's deadline is missed.
	 * 			So, each may react individually on a timing failure. The pointer may be NULL,
	 * 			then nothing happens on missing the deadline. The deadline is equal to the next execution
	 * 			of the periodic task.
	 */
	void (*deadlineMissedFunc)();
};

#endif /* FRAMEWORK_OSAL_LINUX_PERIODICPOSIXTASK_H_ */

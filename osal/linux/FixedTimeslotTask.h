#ifndef FSFW_OSAL_LINUX_FIXEDTIMESLOTTASK_H_
#define FSFW_OSAL_LINUX_FIXEDTIMESLOTTASK_H_

#include "PosixThread.h"
#include "../../tasks/FixedTimeslotTaskIF.h"
#include "../../tasks/FixedSlotSequence.h"
#include <pthread.h>

class FixedTimeslotTask: public FixedTimeslotTaskIF, public PosixThread {
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
	FixedTimeslotTask(const char* name_, int priority_, size_t stackSize_,
			uint32_t periodMs_);
	virtual ~FixedTimeslotTask();

	virtual ReturnValue_t startTask();

	virtual ReturnValue_t sleepFor(uint32_t ms);

	virtual uint32_t getPeriodMs() const;

	virtual ReturnValue_t addSlot(object_id_t componentId, uint32_t slotTimeMs,
			int8_t executionStep);

	virtual ReturnValue_t checkSequence() const;

	/**
	 * This static function can be used as #deadlineMissedFunc.
	 * It counts missedDeadlines and prints the number of missed deadlines every 10th time.
	 */
	static void missedDeadlineCounter();

	/**
	 * A helper variable to count missed deadlines.
	 */
	static uint32_t deadlineMissedCount;

protected:
	/**
	 * @brief	This function holds the main functionality of the thread.
	 * @details
	 * Holding the main functionality of the task, this method is most important.
	 * It links the functionalities provided by FixedSlotSequence with the
	 * OS's System Calls to keep the timing of the periods.
	 */
	virtual void taskFunctionality();

private:
	/**
	 * @brief	This is the entry point in a new thread.
	 *
	 * @details
	 * This method, that is the entry point in the new thread and calls
	 * taskFunctionality of the child class. Needs a valid pointer to the
	 * derived class.
	 *
	 * The void* returnvalue is not used yet but could be used to return
	 * arbitrary data.
	 */
	static void* taskEntryPoint(void* arg);
	FixedSlotSequence pst;

	bool started;
};

#endif /* FSFW_OSAL_LINUX_FIXEDTIMESLOTTASK_H_ */

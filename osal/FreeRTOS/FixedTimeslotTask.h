#ifndef FSFW_OSAL_FREERTOS_FIXEDTIMESLOTTASK_H_
#define FSFW_OSAL_FREERTOS_FIXEDTIMESLOTTASK_H_

#include "FreeRTOSTaskIF.h"
#include "../../tasks/FixedSlotSequence.h"
#include "../../tasks/FixedTimeslotTaskIF.h"
#include "../../tasks/Typedef.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class FixedTimeslotTask: public FixedTimeslotTaskIF, public FreeRTOSTaskIF {
public:

	/**
	 * Keep in mind that you need to call before vTaskStartScheduler()!
	 * A lot of task parameters are set in "FreeRTOSConfig.h".
	 * @param name Name of the task, lenght limited by configMAX_TASK_NAME_LEN
	 * @param setPriority Number of priorities specified by
	 * configMAX_PRIORITIES. High taskPriority_ number means high priority.
	 * @param setStack	Stack size in words (not bytes!).
	 * Lower limit specified by configMINIMAL_STACK_SIZE
	 * @param overallPeriod		Period in seconds.
	 * @param setDeadlineMissedFunc Callback if a deadline was missed.
	 * @return Pointer to the newly created task.
	 */
	FixedTimeslotTask(TaskName name, TaskPriority setPriority,
			TaskStackSize setStack, TaskPeriod overallPeriod,
			void (*setDeadlineMissedFunc)());

	/**
	 * @brief	The destructor of the class.
	 * @details
	 * The destructor frees all heap memory that was allocated on thread
	 * initialization for the PST and the device handlers. This is done by
	 * calling the PST's destructor.
	 */
	virtual ~FixedTimeslotTask(void);

	ReturnValue_t startTask(void);
	/**
	 * This static function can be used as #deadlineMissedFunc.
	 * It counts missedDeadlines and prints the number of missed deadlines
	 * every 10th time.
	 */
	static void missedDeadlineCounter();
	/**
	 * A helper variable to count missed deadlines.
	 */
	static uint32_t deadlineMissedCount;

	ReturnValue_t addSlot(object_id_t componentId, uint32_t slotTimeMs,
			int8_t executionStep) override;

	uint32_t getPeriodMs() const override;

	ReturnValue_t checkSequence() const override;

	ReturnValue_t sleepFor(uint32_t ms) override;

	TaskHandle_t getTaskHandle() override;

protected:
	bool started;
	TaskHandle_t handle;

	FixedSlotSequence pst;

	/**
	 * @brief	This attribute holds a function pointer that is executed when
	 *          a deadline was missed.
	 * @details
	 * Another function may be announced to determine the actions to perform
	 * when a deadline was missed. Currently, only one function for missing
	 * any deadline is allowed. If not used, it shall be declared NULL.
	 */
	void (*deadlineMissedFunc)(void);
	/**
	 * @brief	This is the entry point for a new task.
	 * @details
	 * This method starts the task by calling taskFunctionality(), as soon as
	 * all requirements (task scheduler has started and startTask()
	 * has been called) are met.
	 */
	static void taskEntryPoint(void* argument);

	/**
	 * @brief	This function holds the main functionality of the thread.
	 * @details
	 * Core function holding the main functionality of the task
	 * It links the functionalities provided by FixedSlotSequence with the
	 * OS's System Calls to keep the timing of the periods.
	 */
	void taskFunctionality(void);

	void checkMissedDeadline(const TickType_t xLastWakeTime,
	        const TickType_t interval);
	void handleMissedDeadline();
};

#endif /* FSFW_OSAL_FREERTOS_FIXEDTIMESLOTTASK_H_ */

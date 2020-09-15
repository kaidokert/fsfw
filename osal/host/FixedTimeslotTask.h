#ifndef FRAMEWORK_OSAL_HOST_FIXEDTIMESLOTTASK_H_
#define FRAMEWORK_OSAL_HOST_FIXEDTIMESLOTTASK_H_

#include "../../objectmanager/ObjectManagerIF.h"
#include "../../tasks/FixedSlotSequence.h"
#include "../../tasks/FixedTimeslotTaskIF.h"
#include "../../tasks/Typedef.h"

#include <vector>
#include <thread>
#include <condition_variable>
#include <atomic>

class ExecutableObjectIF;

/**
 * @brief   This class represents a task for periodic activities with multiple
 *          steps and strict timeslot requirements for these steps.
 * @details
 * @ingroup task_handling
 */
class FixedTimeslotTask: public FixedTimeslotTaskIF {
public:
    /**
     * @brief   Standard constructor of the class.
     * @details
     * The class is initialized without allocated objects. These need to be
     * added with #addComponent.
     * @param priority
     * @param stack_size
     * @param setPeriod
     * @param setDeadlineMissedFunc
     * The function pointer to the deadline missed function that shall be
     * assigned.
     */
    FixedTimeslotTask(const char *name, TaskPriority setPriority,
            TaskStackSize setStack, TaskPeriod setPeriod,
            void (*setDeadlineMissedFunc)());
    /**
     * @brief   Currently, the executed object's lifetime is not coupled with
     *          the task object's lifetime, so the destructor is empty.
     */
    virtual ~FixedTimeslotTask(void);

    /**
     * @brief   The method to start the task.
     * @details The method starts the task with the respective system call.
     *          Entry point is the taskEntryPoint method described below.
     *          The address of the task object is passed as an argument
     *          to the system call.
     */
    ReturnValue_t startTask(void);

    /**
     * Add timeslot to the polling sequence table.
     * @param componentId
     * @param slotTimeMs
     * @param executionStep
     * @return
     */
    ReturnValue_t addSlot(object_id_t componentId,
                   uint32_t slotTimeMs, int8_t executionStep);

    ReturnValue_t checkSequence() const override;

    uint32_t getPeriodMs() const;

    ReturnValue_t sleepFor(uint32_t ms);

protected:
    using chron_ms = std::chrono::milliseconds;

    bool started;
    //!< Typedef for the List of objects.
    typedef std::vector<ExecutableObjectIF*> ObjectList;
    std::thread mainThread;
    std::atomic<bool> terminateThread = false;

    //! Polling sequence table which contains the object to execute
    //! and information like the timeslots and the passed execution step.
    FixedSlotSequence pollingSeqTable;

    std::condition_variable initCondition;
    std::mutex initMutex;
    std::string taskName;
    /**
     * @brief   The period of the task.
     * @details
     * The period determines the frequency of the task's execution.
     * It is expressed in clock ticks.
     */
    TaskPeriod period;

    /**
     * @brief   The pointer to the deadline-missed function.
     * @details
     * This pointer stores the function that is executed if the task's deadline
     * is missed. So, each may react individually on a timing failure.
     * The pointer may be NULL, then nothing happens on missing the deadline.
     * The deadline is equal to the next execution of the periodic task.
     */
    void (*deadlineMissedFunc)(void);
    /**
     * @brief   This is the function executed in the new task's context.
     * @details
     * It converts the argument back to the thread object type and copies the
     * class instance to the task context.
     * The taskFunctionality method is called afterwards.
     * @param   A pointer to the task object itself is passed as argument.
     */

    void taskEntryPoint(void* argument);
    /**
     * @brief   The function containing the actual functionality of the task.
     * @details
     * The method sets and starts the task's period, then enters a loop that is
     * repeated as long as the isRunning attribute is true. Within the loop,
     * all performOperation methods of the added objects are called. Afterwards
     * the checkAndRestartPeriod system call blocks the task until the next
     *  period. On missing the deadline, the deadlineMissedFunction is executed.
     */
    void taskFunctionality(void);

    bool delayForInterval(chron_ms * previousWakeTimeMs,
            const chron_ms interval);
};



#endif /* FRAMEWORK_OSAL_HOST_FIXEDTIMESLOTTASK_H_ */

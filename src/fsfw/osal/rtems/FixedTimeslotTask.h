#ifndef FSFW_OSAL_RTEMS_FIXEDTIMESLOTTASK_H_
#define FSFW_OSAL_RTEMS_FIXEDTIMESLOTTASK_H_

#include "RTEMSTaskBase.h"
#include "fsfw/tasks/FixedSlotSequence.h"
#include "fsfw/tasks/FixedTimeslotTaskBase.h"

class FixedTimeslotTask : public FixedTimeslotTaskBase, public RTEMSTaskBase {
 public:
  /**
   * @brief	The standard constructor of the class.
   * @details
   * This is the general constructor of the class. In addition to the TaskBase parameters,
   * the following variables are passed:
   * @param	setDeadlineMissedFunc   The function pointer to the deadline missed function
   *                                  that shall be assigned.
   * @param	getPst The object id of the completely initialized polling sequence.
   */
  FixedTimeslotTask(const char *name, rtems_task_priority setPriority, size_t setStackSize,
                    TaskPeriod overallPeriod, TaskDeadlineMissedFunction dlmFunc);

  /**
   * @brief	The destructor of the class.
   * @details
   * The destructor frees all heap memory that was allocated on thread initialization
   * for the PST andthe device handlers. This is done by calling the PST's destructor.
   */
  ~FixedTimeslotTask() override;

  ReturnValue_t startTask(void);

  ReturnValue_t sleepFor(uint32_t ms) override;

 protected:
  /**
   * @brief id of the associated OS period
   */
  rtems_id periodId;
  /**
   * @brief	This is the entry point in a new polling thread.
   * @details	This method is the entry point in the new thread
   */
  static rtems_task taskEntryPoint(rtems_task_argument argument);

  /**
   * @brief	This function holds the main functionality of the thread.
   * @details
   * Holding the main functionality of the task, this method is most important.
   * It links the functionalities provided by FixedSlotSequence with the OS's system calls to
   * keep the timing of the periods.
   */
  [[noreturn]] void taskFunctionality();
};

#endif /* FSFW_OSAL_RTEMS_FIXEDTIMESLOTTASK_H_ */

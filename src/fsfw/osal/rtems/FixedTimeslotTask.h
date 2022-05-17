#ifndef FSFW_OSAL_RTEMS_FIXEDTIMESLOTTASK_H_
#define FSFW_OSAL_RTEMS_FIXEDTIMESLOTTASK_H_

#include "../../tasks/FixedSlotSequence.h"
#include "../../tasks/FixedTimeslotTaskIF.h"
#include "RTEMSTaskBase.h"

class FixedTimeslotTask : public RTEMSTaskBase, public FixedTimeslotTaskIF {
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
                    uint32_t overallPeriod, void (*setDeadlineMissedFunc)());

  /**
   * @brief	The destructor of the class.
   * @details
   * The destructor frees all heap memory that was allocated on thread initialization
   * for the PST andthe device handlers. This is done by calling the PST's destructor.
   */
  virtual ~FixedTimeslotTask(void);

  ReturnValue_t startTask(void);
  /**
   * This static function can be used as #deadlineMissedFunc.
   * It counts missedDeadlines and prints the number of missed deadlines every 10th time.
   */
  static void missedDeadlineCounter();
  /**
   * A helper variable to count missed deadlines.
   */
  static uint32_t deadlineMissedCount;

  ReturnValue_t addSlot(object_id_t componentId, uint32_t slotTimeMs, int8_t executionStep);

  uint32_t getPeriodMs() const;

  ReturnValue_t checkSequence() const;

  ReturnValue_t sleepFor(uint32_t ms);

 protected:
  /**
   * @brief id of the associated OS period
   */
  rtems_id periodId;

  FixedSlotSequence pst;

  /**
   * @brief	This attribute holds a function pointer that is executed when a deadline was missed.
   *
   * @details
   * Another function may be announced to determine the actions to perform when a deadline
   * was missed. Currently, only one function for missing any deadline is allowed.
   * If not used, it shall be declared NULL.
   */
  void (*deadlineMissedFunc)(void) = nullptr;
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
  void taskFunctionality(void);
};

#endif /* FSFW_OSAL_RTEMS_FIXEDTIMESLOTTASK_H_ */

#ifndef FSFW_OSAL_LINUX_FIXEDTIMESLOTTASK_H_
#define FSFW_OSAL_LINUX_FIXEDTIMESLOTTASK_H_

#include <pthread.h>

#include "PosixThread.h"
#include "fsfw/tasks/FixedSlotSequence.h"
#include "fsfw/tasks/FixedTimeslotTaskBase.h"
#include "fsfw/tasks/definitions.h"

class FixedTimeslotTask : public FixedTimeslotTaskBase {
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
  FixedTimeslotTask(const char* name_, TaskPriority priority_, size_t stackSize_,
                    TaskPeriod periodSeconds_, TaskDeadlineMissedFunction dlmFunc_);
  ~FixedTimeslotTask() override = default;

  ReturnValue_t startTask() override;

  ReturnValue_t sleepFor(uint32_t ms) override;

 protected:
  /**
   * @brief	This function holds the main functionality of the thread.
   * @details
   * Holding the main functionality of the task, this method is most important.
   * It links the functionalities provided by FixedSlotSequence with the
   * OS's System Calls to keep the timing of the periods.
   */
  [[noreturn]] virtual void taskFunctionality();

 private:
  PosixThread posixThread;
  bool started;

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
};

#endif /* FSFW_OSAL_LINUX_FIXEDTIMESLOTTASK_H_ */

#ifndef FRAMEWORK_OSAL_LINUX_PERIODICPOSIXTASK_H_
#define FRAMEWORK_OSAL_LINUX_PERIODICPOSIXTASK_H_

#include <vector>

#include "PosixThread.h"
#include "fsfw/objectmanager/ObjectManagerIF.h"
#include "fsfw/tasks/ExecutableObjectIF.h"
#include "fsfw/tasks/PeriodicTaskBase.h"
#include "fsfw/tasks/PeriodicTaskIF.h"

class PeriodicPosixTask : public PeriodicTaskBase {
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
  PeriodicPosixTask(const char* name_, int priority_, size_t stackSize_, TaskPeriod period_,
                    TaskDeadlineMissedFunction dlmFunc_);
  ~PeriodicPosixTask() override = default;

  /**
   * @brief	The method to start the task.
   * @details	The method starts the task with the respective system call.
   * 			Entry point is the taskEntryPoint method described below.
   * 			The address of the task object is passed as an argument
   * 			to the system call.
   */
  ReturnValue_t startTask() override;

  ReturnValue_t sleepFor(uint32_t ms) override;

 private:
  PosixThread posixThread;

  /**
   * @brief Flag to indicate that the task was started and is allowed to run
   */
  bool started;

  /**
   * @brief	The function containing the actual functionality of the task.
   * @details	The method sets and starts
   * 			the task's period, then enters a loop that is repeated indefinitely. Within
   * the loop, all performOperation methods of the added objects are called. Afterwards the task
   * will be blocked until the next period. On missing the deadline, the deadlineMissedFunction is
   * executed.
   */
  [[noreturn]] virtual void taskFunctionality();
  /**
   * @brief	This is the entry point in a new thread.
   *
   * @details	This method, that is the entry point in the new thread and calls taskFunctionality
   * of the child class. Needs a valid pointer to the derived class.
   */
  static void* taskEntryPoint(void* arg);
};

#endif /* FRAMEWORK_OSAL_LINUX_PERIODICPOSIXTASK_H_ */

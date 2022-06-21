#ifndef FRAMEWORK_OSAL_HOST_PERIODICTASK_H_
#define FRAMEWORK_OSAL_HOST_PERIODICTASK_H_

#include <atomic>
#include <condition_variable>
#include <thread>
#include <vector>

#include "fsfw/objectmanager/ObjectManagerIF.h"
#include "fsfw/tasks/PeriodicTaskBase.h"
#include "fsfw/tasks/definitions.h"

class ExecutableObjectIF;

/**
 * @brief 	This class represents a specialized task for
 * 			periodic activities of multiple objects.
 * @details
 *
 * @ingroup task_handling
 */
class PeriodicTask : public PeriodicTaskBase {
 public:
  /**
   * @brief	Standard constructor of the class.
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
  PeriodicTask(const char* name, TaskPriority setPriority, TaskStackSize setStack,
               TaskPeriod setPeriod, TaskDeadlineMissedFunction dlmFunc);
  /**
   * @brief	Currently, the executed object's lifetime is not coupled with
   * 			the task object's lifetime, so the destructor is empty.
   */
  ~PeriodicTask() override;

  /**
   * @brief	The method to start the task.
   * @details	The method starts the task with the respective system call.
   * 			Entry point is the taskEntryPoint method described below.
   * 			The address of the task object is passed as an argument
   * 			to the system call.
   */
  ReturnValue_t startTask() override;

  ReturnValue_t sleepFor(uint32_t ms) override;

 protected:
  using chron_ms = std::chrono::milliseconds;
  bool started;
  std::thread mainThread;
  std::atomic<bool> terminateThread{false};

  std::condition_variable initCondition;
  std::mutex initMutex;
  std::string taskName;

  /**
   * @brief	This is the function executed in the new task's context.
   * @details
   * It converts the argument back to the thread object type and copies the
   * class instance to the task context.
   * The taskFunctionality method is called afterwards.
   * @param	A pointer to the task object itself is passed as argument.
   */

  void taskEntryPoint(void* argument);
  /**
   * @brief	The function containing the actual functionality of the task.
   * @details
   * The method sets and starts the task's period, then enters a loop that is
   * repeated as long as the isRunning attribute is true. Within the loop,
   * all performOperation methods of the added objects are called. Afterwards
   * the checkAndRestartPeriod system call blocks the task until the next
   *  period. On missing the deadline, the deadlineMissedFunction is executed.
   */
  void taskFunctionality();

  static bool delayForInterval(chron_ms* previousWakeTimeMs, chron_ms interval);
};

#endif /* FRAMEWORK_OSAL_HOST_PERIODICTASK_H_ */

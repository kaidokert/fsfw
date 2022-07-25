#ifndef FSFW_OSAL_FREERTOS_PERIODICTASK_H_
#define FSFW_OSAL_FREERTOS_PERIODICTASK_H_

#include <vector>

#include "FreeRTOS.h"
#include "FreeRTOSTaskIF.h"
#include "fsfw/objectmanager/ObjectManagerIF.h"
#include "fsfw/tasks/PeriodicTaskBase.h"
#include "fsfw/tasks/definitions.h"
#include "task.h"

class ExecutableObjectIF;

/**
 * @brief 	This class represents a specialized task for
 * 			periodic activities of multiple objects.
 * @ingroup task_handling
 */
class PeriodicTask : public PeriodicTaskBase, public FreeRTOSTaskIF {
 public:
  /**
   * Keep in Mind that you need to call before this vTaskStartScheduler()!
   * A lot of task parameters are set in "FreeRTOSConfig.h".
   * @details
   * The class is initialized without allocated objects.
   * These need to be added with #addComponent.
   * @param priority
   * Sets the priority of a task. Values depend on freeRTOS configuration,
   * high number means high priority.
   * @param stack_size
   * The stack size reserved by the operating system for the task.
   * @param setPeriod
   * The length of the period with which the task's
   * functionality will be executed. It is expressed in clock ticks.
   * @param setDeadlineMissedFunc
   * The function pointer to the deadline missed function that shall
   * be assigned.
   */
  PeriodicTask(TaskName name, TaskPriority setPriority, TaskStackSize setStack,
               TaskPeriod setPeriod, TaskDeadlineMissedFunction deadlineMissedFunc);
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

  TaskHandle_t getTaskHandle() override;

 protected:
  bool started;
  TaskHandle_t handle;

  /**
   * @brief	This is the function executed in the new task's context.
   * @details
   * It converts the argument back to the thread object type and copies the
   * class instance to the task context. The taskFunctionality method is
   * called afterwards.
   * @param	A pointer to the task object itself is passed as argument.
   */

  static void taskEntryPoint(void* argument);
  /**
   * @brief	The function containing the actual functionality of the task.
   * @details
   * The method sets and starts the task's period, then enters a loop that is
   * repeated as long as the isRunning attribute is true. Within the loop,
   * all performOperation methods of the added objects are called.
   * Afterwards the checkAndRestartPeriod system call blocks the task until
   * the next period.
   * On missing the deadline, the deadlineMissedFunction is executed.
   */
  [[noreturn]] void taskFunctionality();

  void handleMissedDeadline();
};

#endif /* FSFW_OSAL_FREERTOS_PERIODICTASK_H_ */

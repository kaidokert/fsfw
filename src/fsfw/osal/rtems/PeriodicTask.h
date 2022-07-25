#ifndef FSFW_OSAL_RTEMS_PERIODICTASK_H_
#define FSFW_OSAL_RTEMS_PERIODICTASK_H_

#include <vector>

#include "RTEMSTaskBase.h"
#include "fsfw/objectmanager/ObjectManagerIF.h"
#include "fsfw/tasks/PeriodicTaskBase.h"
#include "fsfw/tasks/PeriodicTaskIF.h"

class ExecutableObjectIF;

/**
 * @brief This class represents a specialized task for periodic activities of multiple objects.
 *
 * @details MultiObjectTask is an extension to ObjectTask in the way that it is able to execute
 * 			multiple objects that implement the ExecutableObjectIF interface. The
 * objects must be added prior to starting the task.
 * @author  baetz
 * @ingroup task_handling
 */
class PeriodicTask : public PeriodicTaskBase, public RTEMSTaskBase {
 public:
  /**
   * @brief	Standard constructor of the class.
   * @details	The class is initialized without allocated objects. These need to be added
   * 			with #addObject.
   * 			In the underlying TaskBase class, a new operating system task is created.
   * 			In addition to the TaskBase parameters, the period, the pointer to the
   * 			aforementioned initialization function and an optional "deadline-missed"
   * 			function pointer is passed.
   * @param priority		Sets the priority of a task. Values range from a low 0 to a high 99.
   * @param stack_size	The stack size reserved by the operating system for the task.
   * @param setPeriod 	The length of the period with which the task's functionality will be
   * 						executed. It is expressed in clock ticks.
   * @param setDeadlineMissedFunc	The function pointer to the deadline missed function
   * 								that shall be assigned.
   */
  PeriodicTask(const char *name, rtems_task_priority setPriority, size_t setStack,
               TaskPeriod setPeriod, TaskDeadlineMissedFunction dlmFunc);
  /**
   * @brief	Currently, the executed object's lifetime is not coupled with the task object's
   * 			lifetime, so the destructor is empty.
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
  /**
   * @brief	The period of the task.
   * @details	The period determines the frequency of the task's execution. It is expressed in
   * clock ticks.
   */
  rtems_interval periodTicks;
  /**
   * @brief id of the associated OS period
   */
  rtems_id periodId = 0;

  /**
   * @brief	This is the function executed in the new task's context.
   * @details	It converts the argument back to the thread object type and copies the class
   * instance to the task context. The taskFunctionality method is called afterwards.
   * @param	A pointer to the task object itself is passed as argument.
   */
  static rtems_task taskEntryPoint(rtems_task_argument argument);
  /**
   * @brief	The function containing the actual functionality of the task.
   * @details	The method sets and starts
   * 			the task's period, then enters a loop that is repeated as long as the
   * isRunning attribute is true. Within the loop, all performOperation methods of the added objects
   * are called. Afterwards the checkAndRestartPeriod system call blocks the task until the next
   * period. On missing the deadline, the deadlineMissedFunction is executed.
   */
  [[noreturn]] void taskFunctionality();
};

#endif /* FSFW_OSAL_RTEMS_PERIODICTASK_H_ */

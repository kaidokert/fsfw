#ifndef FSFW_OSAL_FREERTOS_FIXEDTIMESLOTTASK_H_
#define FSFW_OSAL_FREERTOS_FIXEDTIMESLOTTASK_H_

#include "FreeRTOS.h"
#include "FreeRTOSTaskIF.h"
#include "fsfw/tasks/FixedSlotSequence.h"
#include "fsfw/tasks/FixedTimeslotTaskBase.h"
#include "fsfw/tasks/definitions.h"
#include "task.h"

class FixedTimeslotTask : public FixedTimeslotTaskBase, public FreeRTOSTaskIF {
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
  FixedTimeslotTask(TaskName name, TaskPriority setPriority, TaskStackSize setStack,
                    TaskPeriod overallPeriod, TaskDeadlineMissedFunction dlmFunc);

  /**
   * @brief	The destructor of the class.
   * @details
   * The destructor frees all heap memory that was allocated on thread
   * initialization for the PST and the device handlers. This is done by
   * calling the PST's destructor.
   */
  ~FixedTimeslotTask() override;

  ReturnValue_t startTask() override;

  ReturnValue_t sleepFor(uint32_t ms) override;

  TaskHandle_t getTaskHandle() override;

 protected:
  bool started;
  TaskHandle_t handle;

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
  [[noreturn]] void taskFunctionality();

  void handleMissedDeadline();
};

#endif /* FSFW_OSAL_FREERTOS_FIXEDTIMESLOTTASK_H_ */

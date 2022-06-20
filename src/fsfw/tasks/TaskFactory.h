#ifndef FSFW_TASKS_TASKFACTORY_H_
#define FSFW_TASKS_TASKFACTORY_H_

#include <cstdlib>

#include "FixedTimeslotTaskIF.h"
#include "definitions.h"

/**
 * Singleton Class that produces Tasks.
 */
class TaskFactory {
 public:
  virtual ~TaskFactory();
  /**
   * Returns the single instance of TaskFactory.
   * The implementation of #instance is found in its subclasses.
   * Thus, we choose link-time variability of the  instance.
   */
  static TaskFactory* instance();

  /**
   * Creates a new periodic task and returns the interface pointer.
   * @param name_ Name of the task
   * @param taskPriority_
   * Priority of the task. This value might have different ranges for the various OSALs.
   *  - Linux     Value ranging from 0 to 99 with 99 being the highest value.
   *  - Host      For Windows, the value can be retrieved by using the #tasks::makeWinPriority
   *              function. For Linux, same priority system as specified above. MacOS not tested
   *              yet
   *  - FreeRTOS  Value depends on the FreeRTOS configuration, higher number means higher priority
   *  - RTEMS     Values ranging from 0 to 99 with 99 being the highest value.
   *
   * @param stackSize_ Stack Size of the task
   * This value might have different recommended ranges for the various OSALs.
   *  - Linux     Lowest limit is the PeriodicTaskIF::MINIMUM_STACK_SIZE value
   *  - Host      Value is ignored for now because the C++ threading abstraction layer is used.
   *  - FreeRTOS  Stack size in bytes. It is recommended to specify at least 1kB of stack for
   *              FSFW tasks, but the lowest possible size is specified in the
   *              FreeRTOSConfig.h file.
   *  - RTEMS     Lowest limit is specified the PeriodicTaskIF::MINIMUM_STACK_SIZE value.
   *
   * @param period_ Period of the task
   *
   * @param deadLineMissedFunction_ Function to be called if a deadline was missed
   * @return PeriodicTaskIF* Pointer to the newly created Task
   */
  PeriodicTaskIF* createPeriodicTask(TaskName name_, TaskPriority taskPriority_,
                                     TaskStackSize stackSize_, TaskPeriod periodInSeconds_,
                                     TaskDeadlineMissedFunction deadLineMissedFunction_);

  /**
   * The meaning for the variables for fixed timeslot tasks is the same as for periodic tasks.
   * See #createPeriodicTask documentation.
   * @param name_ Name of the task
   * @param taskPriority_ Priority of the task
   * @param stackSize_ Stack Size of the task
   * @param period_ Period of the task
   * @param deadLineMissedFunction_ Function to be called if a deadline was missed
   * @return FixedTimeslotTaskIF* Pointer to the newly created Task
   */
  FixedTimeslotTaskIF* createFixedTimeslotTask(TaskName name_, TaskPriority taskPriority_,
                                               TaskStackSize stackSize_,
                                               TaskPeriod periodInSeconds_,
                                               TaskDeadlineMissedFunction deadLineMissedFunction_);

  /**
   * Function to be called to delete a task
   * @param task The pointer to the task that shall be deleted,
   * nullptr specifies current Task
   * @return Success of deletion
   */
  static ReturnValue_t deleteTask(PeriodicTaskIF* task = nullptr);

  /**
   * Function to be called to delay current task
   * @param delay The delay in milliseconds
   * @return Success of deletion
   */
  static ReturnValue_t delayTask(uint32_t delayMs);

  /**
   * OS specific implementation to print deadline. In most cases, there is a OS specific
   * way to retrieve the task name and print it out as well.
   */
  static void printMissedDeadline();

 private:
  /**
   * External instantiation is not allowed.
   */
  TaskFactory();
  static TaskFactory* factoryInstance;
};

#endif /* FSFW_TASKS_TASKFACTORY_H_ */

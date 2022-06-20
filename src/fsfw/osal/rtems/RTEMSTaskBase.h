#ifndef FSFW_OSAL_RTEMS_RTEMSTASKBASE_H_
#define FSFW_OSAL_RTEMS_RTEMSTASKBASE_H_

#include "../../tasks/PeriodicTaskIF.h"
#include "RtemsBasic.h"

/**
 * @brief	This is the basic task handling class for rtems.
 *
 * @details Task creation base class for rtems.
 */
class RTEMSTaskBase {
 protected:
  /**
   * @brief	The class stores the task id it got assigned from the operating system in this
   * attribute. If initialization fails, the id is set to zero.
   */
  rtems_id id;

 public:
  /**
   * @brief	The constructor creates and initializes a task.
   * @details	This is accomplished by using the operating system call to create a task. The name
   * is created automatically with the help od taskCounter. Priority and stack size are adjustable,
   * all other attributes are set with default values.
   * @param priority		Sets the priority of a task. Values range from a low 0 to a high 99.
   * @param stack_size	The stack size reserved by the operating system for the task.
   * @param nam			The name of the Task, as a null-terminated String. Currently max 4
   * chars supported (excluding Null-terminator), rest will be truncated
   */
  RTEMSTaskBase(rtems_task_priority priority, size_t stack_size, const char *name);
  /**
   * @brief	In the destructor, the created task is deleted.
   */
  virtual ~RTEMSTaskBase();
  /**
   * @brief	This method returns the task id of this class.
   */
  rtems_id getId() const;

  static ReturnValue_t sleepFor(uint32_t ms);
  static ReturnValue_t setAndStartPeriod(rtems_interval period, rtems_id *periodId);
  static rtems_status_code restartPeriod(rtems_interval period, rtems_id periodId);

 private:
  static ReturnValue_t convertReturnCode(rtems_status_code inValue);
};

#endif /* FSFW_OSAL_RTEMS_RTEMSTASKBASE_H_ */

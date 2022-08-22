#ifndef FRAMEWORK_TASKS_EXECUTABLEOBJECTIF_H_
#define FRAMEWORK_TASKS_EXECUTABLEOBJECTIF_H_

class PeriodicTaskIF;

#include <cstring>

#include "../returnvalues/returnvalue.h"
/**
 * @brief	The interface provides a method to execute objects within a task.
 * @details	The performOperation method, that is required by the interface is
 * 			executed cyclically within a task context.
 * @author  Bastian Baetz
 */
class ExecutableObjectIF {
 public:
  /**
   * @brief	This is the empty virtual destructor as required for C++ interfaces.
   */
  virtual ~ExecutableObjectIF() {}
  /**
   * @brief	The performOperation method is executed in a task.
   * @details	There are no restrictions for calls within this method, so any
   * 			other member of the class can be used.
   * @return	Currently, the return value is ignored.
   */
  virtual ReturnValue_t performOperation(uint8_t operationCode = 0) = 0;

  /**
   * @brief Function called during setup assignment of object to task
   * @details
   * Has to be called from the function that assigns the object to a task and
   * enables the object implementation to overwrite this function and get
   * a reference to the executing task
   * @param task_ Pointer to the taskIF of this task
   */
  virtual void setTaskIF(PeriodicTaskIF* task_){};

  /**
   * This function should be called after the object was assigned to a
   * specific task.
   *
   * Example: Can be used to get task execution frequency.
   * The task is created after initialize() and the object ctors have been
   * called so the execution frequency can't be cached in initialize()
   * @return
   */
  virtual ReturnValue_t initializeAfterTaskCreation() { return returnvalue::OK; }
};

#endif /* FRAMEWORK_TASKS_EXECUTABLEOBJECTIF_H_ */

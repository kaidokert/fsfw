#ifndef FRAMEWORK_TASK_PERIODICTASKIF_H_
#define FRAMEWORK_TASK_PERIODICTASKIF_H_

#include <cstddef>

#include "../objectmanager/SystemObjectIF.h"
#include "../timemanager/Clock.h"
class ExecutableObjectIF;

/**
 * New version of TaskIF
 * Follows RAII principles, i.e. there's no create or delete method.
 * Minimalistic.
 */
class PeriodicTaskIF {
 public:
  static const size_t MINIMUM_STACK_SIZE;
  /**
   * @brief	A virtual destructor as it is mandatory for interfaces.
   */
  virtual ~PeriodicTaskIF() {}
  /**
   * @brief	With the startTask method, a created task can be started
   *          for the first time.
   */
  virtual ReturnValue_t startTask() = 0;

  /**
   * Add a component (object) to a periodic task.
   * @param object
   * Add an object to the task. The object needs to implement ExecutableObjectIF
   * @return
   */
  virtual ReturnValue_t addComponent(object_id_t object, uint8_t opCode = 0) {
    return HasReturnvaluesIF::RETURN_FAILED;
  };

  /**
   * Add an object to a periodic task.
   * @param object
   * Add an object to the task.
   * @return
   */
  virtual ReturnValue_t addComponent(ExecutableObjectIF* object, uint8_t opCode = 0) {
    return HasReturnvaluesIF::RETURN_FAILED;
  };

  virtual ReturnValue_t sleepFor(uint32_t ms) = 0;

  virtual uint32_t getPeriodMs() const = 0;

  virtual bool isEmpty() const = 0;
};

#endif /* PERIODICTASKIF_H_ */

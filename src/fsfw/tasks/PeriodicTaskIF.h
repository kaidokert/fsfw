#ifndef FRAMEWORK_TASK_PERIODICTASKIF_H_
#define FRAMEWORK_TASK_PERIODICTASKIF_H_

#include <cstddef>

#include "fsfw/objectmanager/SystemObjectIF.h"
#include "fsfw/tasks/ExecutableObjectIF.h"

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
  virtual ~PeriodicTaskIF() = default;
  /**
   * @brief	With the startTask method, a created task can be started
   *          for the first time.
   */
  virtual ReturnValue_t startTask() = 0;

  /**
   * Adds an object to the list of objects to be executed.
   * The objects are executed in the order added. The object needs to implement
   * ExecutableObjectIF
   * @param object Id of the object to add.
   * @return returnvalue::OK on success, returnvalue::FAILED if the object could not be added.
   */
  virtual ReturnValue_t addComponent(object_id_t object, uint8_t opCode) = 0;
  virtual ReturnValue_t addComponent(object_id_t object) { return addComponent(object, 0); };

  /**
   * Adds an object to the list of objects to be executed.
   * The objects are executed in the order added.
   * @param object pointer to the object to add.
   * @return returnvalue::OK on success, returnvalue::FAILED if the object could not be added.
   */
  virtual ReturnValue_t addComponent(ExecutableObjectIF* object, uint8_t opCode) = 0;
  virtual ReturnValue_t addComponent(ExecutableObjectIF* object) { return addComponent(object, 0); }

  virtual ReturnValue_t sleepFor(uint32_t ms) = 0;

  [[nodiscard]] virtual uint32_t getPeriodMs() const = 0;

  [[nodiscard]] virtual bool isEmpty() const = 0;
};

#endif /* FRAMEWORK_TASK_PERIODICTASKIF_H_ */

#ifndef FSFW_SRC_FSFW_TASKS_PERIODICTASKBASE_H_
#define FSFW_SRC_FSFW_TASKS_PERIODICTASKBASE_H_

#include <cstdint>
#include <vector>

#include "fsfw/tasks/PeriodicTaskIF.h"
#include "fsfw/tasks/definitions.h"

class ExecutableObjectIF;

class PeriodicTaskBase : public PeriodicTaskIF {
 public:
  explicit PeriodicTaskBase(TaskPeriod period,
                            TaskDeadlineMissedFunction deadlineMissedFunc = nullptr);

  ReturnValue_t addComponent(object_id_t object, uint8_t opCode) override;
  ReturnValue_t addComponent(ExecutableObjectIF* object, uint8_t opCode) override;

  ReturnValue_t addComponent(object_id_t object) override;
  ReturnValue_t addComponent(ExecutableObjectIF* object) override;

  [[nodiscard]] uint32_t getPeriodMs() const override;

  [[nodiscard]] bool isEmpty() const override;

  ReturnValue_t initObjsAfterTaskCreation();

 protected:
  //! Typedef for the List of objects. Will contain the objects to execute and their respective
  //! operation codes
  using ObjectList = std::vector<std::pair<ExecutableObjectIF*, uint8_t>>;
  /**
   * @brief This attribute holds a list of objects to be executed.
   */
  ObjectList objectList;

  /**
   * @brief Period of task in floating point seconds
   */
  TaskPeriod period;

  /**
   * @brief The pointer to the deadline-missed function.
   * @details
   * This pointer stores the function that is executed if the task's deadline
   * is missed. So, each may react individually on a timing failure.
   * The pointer may be NULL, then nothing happens on missing the deadline.
   * The deadline is equal to the next execution of the periodic task.
   */
  TaskDeadlineMissedFunction dlmFunc = nullptr;
};

#endif /* FSFW_SRC_FSFW_TASKS_PERIODICTASKBASE_H_ */

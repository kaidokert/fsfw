#ifndef FSFW_SRC_FSFW_TASKS_PERIODICTASKBASE_H_
#define FSFW_SRC_FSFW_TASKS_PERIODICTASKBASE_H_

#include "fsfw/tasks/PeriodicTaskIF.h"
#include "fsfw/tasks/definitions.h"
#include <vector>
#include <cstdint>

class ExecutableObjectIF;

class PeriodicTaskBase: public PeriodicTaskIF {
public:
  PeriodicTaskBase(uint32_t periodMs, TaskDeadlineMissedFunction deadlineMissedFunc = nullptr);

  ReturnValue_t addComponent(object_id_t object, uint8_t opCode) override;
  ReturnValue_t addComponent(ExecutableObjectIF* object, uint8_t opCode) override;


  uint32_t getPeriodMs() const override;

  bool isEmpty() const override;

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
   * @brief Period of the task in milliseconds
   */
  uint32_t periodMs;

  /**
   * @brief The pointer to the deadline-missed function.
   * @details   This pointer stores the function that is executed if the task's deadline is missed.
   *            So, each may react individually on a timing failure. The pointer may be
   * NULL, then nothing happens on missing the deadline. The deadline is equal to the next execution
   *            of the periodic task.
   */
  TaskDeadlineMissedFunction deadlineMissedFunc = nullptr;
};



#endif /* FSFW_SRC_FSFW_TASKS_PERIODICTASKBASE_H_ */

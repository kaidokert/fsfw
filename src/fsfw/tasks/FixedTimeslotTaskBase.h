#ifndef FSFW_EXAMPLE_HOSTED_FIXEDTIMESLOTTASKBASE_H
#define FSFW_EXAMPLE_HOSTED_FIXEDTIMESLOTTASKBASE_H

#include "FixedSlotSequence.h"
#include "FixedTimeslotTaskIF.h"
#include "definitions.h"

class FixedTimeslotTaskBase : public FixedTimeslotTaskIF {
 public:
  explicit FixedTimeslotTaskBase(TaskPeriod period, TaskDeadlineMissedFunction dlmFunc = nullptr);
  ~FixedTimeslotTaskBase() override = default;
  ;

 protected:
  /**
   * @brief Period of task in floating point seconds
   */
  TaskPeriod period;

  //! Polling sequence table which contains the object to execute
  //! and information like the timeslots and the passed execution step.
  FixedSlotSequence pollingSeqTable;

  /**
   * @brief The pointer to the deadline-missed function.
   * @details
   * This pointer stores the function that is executed if the task's deadline
   * is missed. So, each may react individually on a timing failure.
   * The pointer may be NULL, then nothing happens on missing the deadline.
   * The deadline is equal to the next execution of the periodic task.
   */
  TaskDeadlineMissedFunction dlmFunc = nullptr;

  ReturnValue_t checkSequence() override;

  [[nodiscard]] uint32_t getPeriodMs() const override;

  [[nodiscard]] bool isEmpty() const override;

  ReturnValue_t addSlot(object_id_t execId, ExecutableObjectIF* componentId, uint32_t slotTimeMs,
                        int8_t executionStep) override;
};

#endif  // FSFW_EXAMPLE_HOSTED_FIXEDTIMESLOTTASKBASE_H

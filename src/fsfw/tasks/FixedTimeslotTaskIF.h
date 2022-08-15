#ifndef FRAMEWORK_TASKS_FIXEDTIMESLOTTASKIF_H_
#define FRAMEWORK_TASKS_FIXEDTIMESLOTTASKIF_H_

#include "PeriodicTaskIF.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/objectmanager/ObjectManagerIF.h"
#include "fsfw/returnvalues/FwClassIds.h"

/**
 * @brief Following the same principle as the base class IF.
 * This is the interface for a Fixed timeslot task
 */
class FixedTimeslotTaskIF : public PeriodicTaskIF {
 public:
  ~FixedTimeslotTaskIF() override = default;

  static constexpr ReturnValue_t SLOT_LIST_EMPTY =
      returnvalue::makeCode(CLASS_ID::FIXED_SLOT_TASK_IF, 0);

  /**
   * Add an object with a slot time and the execution step to the task.
   * The execution step will be passed to the object (e.g. as an operation
   * code in #performOperation)
   * @param componentId
   * @param slotTimeMs
   * @param executionStep
   * @return
   */
  virtual ReturnValue_t addSlot(object_id_t execId, ExecutableObjectIF* obj, uint32_t slotTimeMs,
                                int8_t executionStep) = 0;

  /**
   * Add an object with a slot time and the execution step to the task.
   * The execution step will be passed to the object (e.g. as an operation
   * code in #performOperation)
   * @param componentId
   * @param slotTimeMs
   * @param executionStep
   * @return
   */
  virtual ReturnValue_t addSlot(object_id_t componentId, uint32_t slotTimeMs,
                                int8_t executionStep) {
    auto* execObj = ObjectManager::instance()->get<ExecutableObjectIF>(componentId);
    return addSlot(componentId, execObj, slotTimeMs, executionStep);
  }

  /**
   * Check whether the sequence is valid and perform all other required
   * initialization steps which are needed after task creation
   */
  virtual ReturnValue_t checkSequence() = 0;

  ReturnValue_t addComponent(object_id_t object, uint8_t opCode) override {
    return returnvalue::FAILED;
  }

  ReturnValue_t addComponent(ExecutableObjectIF* object, uint8_t opCode) override {
    return returnvalue::FAILED;
  }
};

#endif /* FRAMEWORK_TASKS_FIXEDTIMESLOTTASKIF_H_ */

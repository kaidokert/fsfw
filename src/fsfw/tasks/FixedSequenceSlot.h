#ifndef FSFW_TASKS_FIXEDSEQUENCESLOT_H_
#define FSFW_TASKS_FIXEDSEQUENCESLOT_H_

#include "ExecutableObjectIF.h"
#include "fsfw/objectmanager/ObjectManagerIF.h"

class PeriodicTaskIF;

/**
 * @brief	This class is the representation of a single polling sequence
 *          table entry.
 * @details
 * The PollingSlot class is the representation of a single polling
 * sequence table entry.
 * @author  baetz
 */
class FixedSequenceSlot {
 public:
  FixedSequenceSlot(object_id_t handlerId, uint32_t setTimeMs, int8_t setSequenceId,
                    ExecutableObjectIF* executableObject, PeriodicTaskIF* executingTask);
  virtual ~FixedSequenceSlot();

  object_id_t handlerId;

  /**
   * @brief	Handler identifies which object is executed in this slot.
   */
  ExecutableObjectIF* executableObject = nullptr;

  /**
   * @brief	This attribute defines when a device handler object is executed.
   * @details
   * The pollingTime attribute identifies the time the handler is
   * executed in ms. It must be smaller than the period length of the
   * polling sequence.
   */
  uint32_t pollingTimeMs;

  /**
   * @brief	This value defines the type of device communication.
   *
   * @details	The state of this value decides what communication routine is
   * 			called in the PST executable or the device handler object.
   */
  uint8_t opcode;

  /**
   * @brief 	Operator overload for the comparison operator to
   * 			allow sorting by polling time.
   * @param fixedSequenceSlot
   * @return
   */
  bool operator<(const FixedSequenceSlot& fixedSequenceSlot) const {
    return pollingTimeMs < fixedSequenceSlot.pollingTimeMs;
  }
};

#endif /* FSFW_TASKS_FIXEDSEQUENCESLOT_H_ */

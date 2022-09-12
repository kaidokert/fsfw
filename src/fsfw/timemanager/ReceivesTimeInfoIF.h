#ifndef FSFW_TIMEMANAGER_RECEIVESTIMEINFOIF_H_
#define FSFW_TIMEMANAGER_RECEIVESTIMEINFOIF_H_

#include "fsfw/ipc/MessageQueueSenderIF.h"

/**
 * This is a Interface for classes that receive timing information
 * with the help of a dedicated message queue.
 */
class ReceivesTimeInfoIF {
 public:
  /**
   * Returns the id of the queue which receives the timing information.
   * @return Queue id of the timing queue.
   */
  [[nodiscard]] virtual MessageQueueId_t getTimeReceptionQueue() const = 0;
  /**
   * Empty virtual destructor.
   */
  virtual ~ReceivesTimeInfoIF() = default;
};

#endif /* FSFW_TIMEMANAGER_RECEIVESTIMEINFOIF_H_ */

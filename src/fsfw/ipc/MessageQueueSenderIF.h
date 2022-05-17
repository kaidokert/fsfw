#ifndef FSFW_IPC_MESSAGEQUEUESENDERIF_H_
#define FSFW_IPC_MESSAGEQUEUESENDERIF_H_

#include "../objectmanager/ObjectManagerIF.h"
#include "MessageQueueIF.h"
#include "MessageQueueMessageIF.h"

class MessageQueueSenderIF {
 public:
  virtual ~MessageQueueSenderIF() {}

  /**
   * Allows sending messages without actually "owning" a message queue.
   * Not sure whether this is actually a good idea.
   */
  static ReturnValue_t sendMessage(MessageQueueId_t sendTo, MessageQueueMessageIF* message,
                                   MessageQueueId_t sentFrom = MessageQueueIF::NO_QUEUE,
                                   bool ignoreFault = false);

 private:
  MessageQueueSenderIF() {}
};

#endif /* FSFW_IPC_MESSAGEQUEUESENDERIF_H_ */

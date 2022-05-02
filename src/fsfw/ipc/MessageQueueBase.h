#ifndef FSFW_SRC_FSFW_IPC_MESSAGEQUEUEBASE_H_
#define FSFW_SRC_FSFW_IPC_MESSAGEQUEUEBASE_H_

#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/ipc/definitions.h>

class MessageQueueBase : public MessageQueueIF {
 public:
  MessageQueueBase(MessageQueueId_t id, MessageQueueId_t defaultDest, MqArgs* mqArgs);
  virtual ~MessageQueueBase();

  // Default implementations for MessageQueueIF where possible
  MessageQueueId_t getLastPartner() const override;
  MessageQueueId_t getId() const override;
  MqArgs& getMqArgs() override;
  void setDefaultDestination(MessageQueueId_t defaultDestination) override;
  MessageQueueId_t getDefaultDestination() const override;
  bool isDefaultDestinationSet() const override;
  ReturnValue_t sendMessage(MessageQueueId_t sendTo, MessageQueueMessageIF* message,
                            bool ignoreFault) override;
  ReturnValue_t sendToDefault(MessageQueueMessageIF* message) override;
  ReturnValue_t reply(MessageQueueMessageIF* message) override;
  ReturnValue_t receiveMessage(MessageQueueMessageIF* message,
                               MessageQueueId_t* receivedFrom) override;
  ReturnValue_t sendToDefaultFrom(MessageQueueMessageIF* message, MessageQueueId_t sentFrom,
                                  bool ignoreFault = false) override;

  // OSAL specific, forward the abstract function
  ReturnValue_t receiveMessage(MessageQueueMessageIF* message) override = 0;
  ReturnValue_t sendMessageFrom(MessageQueueId_t sendTo, MessageQueueMessageIF* message,
                                MessageQueueId_t sentFrom, bool ignoreFault = false) override = 0;

 protected:
  MessageQueueId_t id = MessageQueueIF::NO_QUEUE;
  MessageQueueId_t last = MessageQueueIF::NO_QUEUE;
  MessageQueueId_t defaultDest = MessageQueueIF::NO_QUEUE;
  MqArgs args = {};
};

#endif /* FSFW_SRC_FSFW_IPC_MESSAGEQUEUEBASE_H_ */

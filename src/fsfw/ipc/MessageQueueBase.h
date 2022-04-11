#ifndef FSFW_SRC_FSFW_IPC_MESSAGEQUEUEBASE_H_
#define FSFW_SRC_FSFW_IPC_MESSAGEQUEUEBASE_H_

#include <fsfw/ipc/definitions.h>
#include <fsfw/ipc/MessageQueueIF.h>

class MessageQueueBase: public MessageQueueIF {
public:
  MessageQueueBase(MessageQueueId_t id, MessageQueueId_t defaultDest, MqArgs* mqArgs);
  virtual ~MessageQueueBase();

  // Default implementations for MessageQueueIF where possible
  virtual MessageQueueId_t getLastPartner() const override;
  virtual MessageQueueId_t getId() const override;
  virtual MqArgs* getMqArgs() override;
  virtual void setDefaultDestination(MessageQueueId_t defaultDestination) override;
  virtual MessageQueueId_t getDefaultDestination() const override;
  virtual bool isDefaultDestinationSet() const override;
  virtual ReturnValue_t sendMessage(MessageQueueId_t sendTo, MessageQueueMessageIF* message,
      bool ignoreFault) override;
  virtual ReturnValue_t sendToDefault(MessageQueueMessageIF* message) override;
  virtual ReturnValue_t reply(MessageQueueMessageIF* message) override;
  virtual ReturnValue_t receiveMessage(MessageQueueMessageIF* message,
      MessageQueueId_t* receivedFrom) override;
  virtual ReturnValue_t sendToDefaultFrom(MessageQueueMessageIF* message,
      MessageQueueId_t sentFrom, bool ignoreFault = false) override;

  // OSAL specific, forward the abstract function
  virtual ReturnValue_t receiveMessage(MessageQueueMessageIF* message) = 0;
  virtual ReturnValue_t sendMessageFrom(MessageQueueId_t sendTo, MessageQueueMessageIF* message,
                                        MessageQueueId_t sentFrom, bool ignoreFault = false) = 0;
protected:
  MessageQueueId_t id = MessageQueueIF::NO_QUEUE;
  MessageQueueId_t last = MessageQueueIF::NO_QUEUE;
  MessageQueueId_t defaultDest = MessageQueueIF::NO_QUEUE;
  MqArgs args = {};
};



#endif /* FSFW_SRC_FSFW_IPC_MESSAGEQUEUEBASE_H_ */

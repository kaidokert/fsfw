#ifndef FSFW_UNITTEST_TESTS_MOCKS_MESSAGEQUEUEMOCKBASE_H_
#define FSFW_UNITTEST_TESTS_MOCKS_MESSAGEQUEUEMOCKBASE_H_

#include <cstring>
#include <queue>

#include "CatchDefinitions.h"
#include "fsfw/ipc/CommandMessage.h"
#include "fsfw/ipc/MessageQueueBase.h"
#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/ipc/MessageQueueMessage.h"

class MessageQueueMockBase : public MessageQueueBase {
 public:
  MessageQueueMockBase();

  explicit MessageQueueMockBase(MessageQueueId_t queueId);

  uint8_t messageSentCounter = 0;
  bool messageSent = false;

  bool wasMessageSent(uint8_t* messageSentCounter_ = nullptr, bool resetCounter = true);

  /**
   * Pop a message, clearing it in the process.
   * @return
   */
  ReturnValue_t popMessage();

  ReturnValue_t receiveMessage(MessageQueueMessageIF* message) override;

  ReturnValue_t flush(uint32_t* count) override;
  ReturnValue_t sendMessageFrom(MessageQueueId_t sendTo, MessageQueueMessageIF* message,
                                        MessageQueueId_t sentFrom,
                                        bool ignoreFault = false) override;
  ReturnValue_t reply(MessageQueueMessageIF* message) override;

  void clearMessages(bool clearCommandMessages = true);
 private:
  std::queue<MessageQueueMessage> messagesSentQueue;
};

#endif /* FSFW_UNITTEST_TESTS_MOCKS_MESSAGEQUEUEMOCKBASE_H_ */

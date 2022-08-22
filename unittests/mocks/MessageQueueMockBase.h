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
  MessageQueueMockBase()
      : MessageQueueBase(MessageQueueIF::NO_QUEUE, MessageQueueIF::NO_QUEUE, nullptr) {}

  uint8_t messageSentCounter = 0;
  bool messageSent = false;

  bool wasMessageSent(uint8_t* messageSentCounter = nullptr, bool resetCounter = true) {
    bool tempMessageSent = messageSent;
    messageSent = false;
    if (messageSentCounter != nullptr) {
      *messageSentCounter = this->messageSentCounter;
    }
    if (resetCounter) {
      this->messageSentCounter = 0;
    }
    return tempMessageSent;
  }

  /**
   * Pop a message, clearing it in the process.
   * @return
   */
  ReturnValue_t popMessage() {
    CommandMessage message;
    message.clear();
    return receiveMessage(&message);
  }

  virtual ReturnValue_t receiveMessage(MessageQueueMessageIF* message) override {
    if (messagesSentQueue.empty()) {
      return MessageQueueIF::EMPTY;
    }
    this->last = message->getSender();
    std::memcpy(message->getBuffer(), messagesSentQueue.front().getBuffer(),
                message->getMessageSize());
    messagesSentQueue.pop();
    return returnvalue::OK;
  }
  virtual ReturnValue_t flush(uint32_t* count) { return returnvalue::OK; }
  virtual ReturnValue_t sendMessageFrom(MessageQueueId_t sendTo, MessageQueueMessageIF* message,
                                        MessageQueueId_t sentFrom,
                                        bool ignoreFault = false) override {
    messageSent = true;
    messageSentCounter++;
    MessageQueueMessage& messageRef = *(dynamic_cast<MessageQueueMessage*>(message));
    messagesSentQueue.push(messageRef);
    return returnvalue::OK;
  }

  virtual ReturnValue_t reply(MessageQueueMessageIF* message) override {
    return sendMessageFrom(MessageQueueIF::NO_QUEUE, message, this->getId(), false);
  }

  void clearMessages(bool clearCommandMessages = true) {
    while (not messagesSentQueue.empty()) {
      if (clearCommandMessages) {
        CommandMessage message;
        std::memcpy(message.getBuffer(), messagesSentQueue.front().getBuffer(),
                    message.getMessageSize());
        message.clear();
      }
      messagesSentQueue.pop();
    }
  }

 private:
  std::queue<MessageQueueMessage> messagesSentQueue;
};

#endif /* FSFW_UNITTEST_TESTS_MOCKS_MESSAGEQUEUEMOCKBASE_H_ */

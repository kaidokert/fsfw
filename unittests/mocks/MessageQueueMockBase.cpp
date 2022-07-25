#include "MessageQueueMockBase.h"


MessageQueueMockBase::MessageQueueMockBase()
    : MessageQueueBase(MessageQueueIF::NO_QUEUE, MessageQueueIF::NO_QUEUE, nullptr) {}

MessageQueueMockBase::MessageQueueMockBase(MessageQueueId_t queueId)
    : MessageQueueBase(queueId, MessageQueueIF::NO_QUEUE, nullptr) {}

bool MessageQueueMockBase::wasMessageSent(uint8_t* messageSentCounter_, bool resetCounter) {
  bool tempMessageSent = messageSent;
  messageSent = false;
  if (messageSentCounter_ != nullptr) {
    *messageSentCounter_ = this->messageSentCounter;
  }
  if (resetCounter) {
    this->messageSentCounter = 0;
  }
  return tempMessageSent;
}

ReturnValue_t MessageQueueMockBase::popMessage() {
  CommandMessage message;
  message.clear();
  return receiveMessage(&message);
}

ReturnValue_t MessageQueueMockBase::receiveMessage(MessageQueueMessageIF* message) {
  if (messagesSentQueue.empty()) {
    return MessageQueueIF::EMPTY;
  }
  this->last = message->getSender();
  std::memcpy(message->getBuffer(), messagesSentQueue.front().getBuffer(),
              message->getMessageSize());
  messagesSentQueue.pop();
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t MessageQueueMockBase::flush(uint32_t* count) {
  return HasReturnvaluesIF::RETURN_FAILED;
}

ReturnValue_t MessageQueueMockBase::sendMessageFrom(MessageQueueId_t sendTo,
                                                    MessageQueueMessageIF* message,
                                                    MessageQueueId_t sentFrom, bool ignoreFault) {
  auto iter = sendMap.find(sendTo);
  if (iter == sendMap.end()) {
    sendMap.emplace(sendTo, SendInfo(message, 1));
  } else {
    iter->second.callCount += 1;
    iter->second.msgs.push(message);
  }
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t MessageQueueMockBase::reply(MessageQueueMessageIF* message) {
  return sendMessageFrom(MessageQueueIF::NO_QUEUE, message, this->getId(), false);
}

void MessageQueueMockBase::clearMessages(bool clearCommandMessages) {
  for (const auto& destInfo: sendMap) {
    if (clearCommandMessages) {
      CommandMessage message;
      std::memcpy(message.getBuffer(), destInfo.second.msgs.front()->getBuffer(),
                  message.getMessageSize());
  }
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

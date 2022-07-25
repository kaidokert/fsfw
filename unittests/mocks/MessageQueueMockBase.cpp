#include "MessageQueueMockBase.h"

#include <algorithm>
#include <stdexcept>

MessageQueueMockBase::MessageQueueMockBase()
    : MessageQueueBase(MessageQueueIF::NO_QUEUE, MessageQueueIF::NO_QUEUE, nullptr) {}

MessageQueueMockBase::MessageQueueMockBase(MessageQueueId_t queueId)
    : MessageQueueBase(queueId, MessageQueueIF::NO_QUEUE, nullptr) {}

bool MessageQueueMockBase::wasMessageSent() const {
  return std::any_of(
      sendMap.begin(), sendMap.end(),
      [](const std::pair<MessageQueueId_t, SendInfo>& pair) { return pair.second.callCount > 0; });
}

size_t MessageQueueMockBase::numberOfSentMessage() const {
  size_t callCount = 0;
  for (auto& destInfo : sendMap) {
    callCount += destInfo.second.callCount;
  }
  return callCount;
}

size_t MessageQueueMockBase::numberOfSentMessage(MessageQueueId_t id) const {
  auto iter = sendMap.find(id);
  if (iter == sendMap.end()) {
    return 0;
  }
  return iter->second.callCount;
}

ReturnValue_t MessageQueueMockBase::clearLastReceivedMessage(bool clearCmdMsg) {
  if (receivedMsgs.empty()) {
    return MessageQueueIF::EMPTY;
  }
  if (clearCmdMsg) {
    CommandMessage message;
    std::memcpy(message.getBuffer(), receivedMsgs.front().getBuffer(), message.getMessageSize());
    message.clearCommandMessage();
  }
  receivedMsgs.pop_front();
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t MessageQueueMockBase::receiveMessage(MessageQueueMessageIF* message) {
  if (receivedMsgs.empty()) {
    return MessageQueueIF::EMPTY;
  }
  std::memcpy(message->getBuffer(), receivedMsgs.front().getBuffer(), message->getMessageSize());
  receivedMsgs.pop_front();
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t MessageQueueMockBase::flush(uint32_t* count) {
  return HasReturnvaluesIF::RETURN_FAILED;
}

ReturnValue_t MessageQueueMockBase::sendMessageFrom(MessageQueueId_t sendTo,
                                                    MessageQueueMessageIF* message,
                                                    MessageQueueId_t sentFrom, bool ignoreFault) {
  if (message == nullptr) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  auto iter = sendMap.find(sendTo);
  MessageQueueMessage messageCopy;
  if (iter == sendMap.end()) {
    createMsgCopy(messageCopy, *message);
    sendMap.emplace(sendTo, SendInfo(messageCopy, 1));
  } else {
    iter->second.callCount += 1;
    createMsgCopy(messageCopy, *message);
    iter->second.msgs.push_back(messageCopy);
  }
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t MessageQueueMockBase::reply(MessageQueueMessageIF* message) {
  return sendMessageFrom(MessageQueueIF::NO_QUEUE, message, this->getId(), false);
}

void MessageQueueMockBase::clearMessages(bool clearCommandMessages) {
  if (not clearCommandMessages) {
    sendMap.clear();
    return;
  }
  for (auto& destInfo : sendMap) {
    for (auto& msg : destInfo.second.msgs) {
      CommandMessage message;
      std::memcpy(message.getBuffer(), destInfo.second.msgs.front().getBuffer(),
                  message.getMessageSize());
      message.clear();
      destInfo.second.msgs.pop_front();
      destInfo.second.callCount--;
    }
  }
  sendMap.clear();
}

void MessageQueueMockBase::addReceivedMessage(MessageQueueMessageIF& msg) {
  MessageQueueMessage messageCopy;
  createMsgCopy(messageCopy, msg);
  receivedMsgs.push_back(messageCopy);
}

void MessageQueueMockBase::createMsgCopy(MessageQueueMessageIF& into, MessageQueueMessageIF& from) {
  if (from.getMessageSize() > into.getMaximumDataSize()) {
    throw std::invalid_argument("Passed message does not fit into message copy");
  }
  std::memcpy(into.getBuffer(), from.getBuffer(), from.getMaximumDataSize());
}

ReturnValue_t MessageQueueMockBase::getNextSentMessage(MessageQueueId_t id,
                                                       MessageQueueMessageIF& message) {
  auto iter = sendMap.find(id);
  if (iter == sendMap.end() or iter->second.callCount == 0) {
    return MessageQueueIF::EMPTY;
  }
  createMsgCopy(message, iter->second.msgs.front());
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t MessageQueueMockBase::getNextSentMessage(MessageQueueMessageIF& message) {
  return getNextSentMessage(MessageQueueBase::getDefaultDestination(), message);
}

ReturnValue_t MessageQueueMockBase::clearLastSentMessage(MessageQueueId_t destId,
                                                         bool clearCmdMsg) {
  auto iter = sendMap.find(destId);
  if (iter == sendMap.end()) {
    return MessageQueueIF::EMPTY;
  }
  return clearLastSentMessage(iter, clearCmdMsg);
}

ReturnValue_t MessageQueueMockBase::clearLastSentMessage(bool clearCmdMsg) {
  auto iter = sendMap.find(getDefaultDestination());
  if (iter == sendMap.end()) {
    return MessageQueueIF::EMPTY;
  }
  ReturnValue_t result = clearLastSentMessage(iter, clearCmdMsg);
  clearEmptyEntries();
  return result;
}

ReturnValue_t MessageQueueMockBase::clearLastSentMessage(
    std::map<MessageQueueId_t, SendInfo>::iterator& iter, bool clearCmdMsg) {
  if (clearCmdMsg) {
    CommandMessage message;
    std::memcpy(message.getBuffer(), iter->second.msgs.front().getBuffer(),
                message.getMessageSize());
    message.clear();
  }
  iter->second.msgs.pop_front();
  iter->second.callCount--;
  return HasReturnvaluesIF::RETURN_OK;
}
void MessageQueueMockBase::clearEmptyEntries() {
  for (auto it = sendMap.cbegin(); it != sendMap.cend();) {
    if (it->second.callCount == 0) {
      sendMap.erase(it++);
    } else {
      ++it;
    }
  }
}

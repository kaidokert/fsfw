#include "MessageQueueMock.h"

#include <algorithm>
#include <stdexcept>

MessageQueueMock::MessageQueueMock(MessageQueueId_t queueId)
    : MessageQueueBase(queueId, MessageQueueIF::NO_QUEUE, nullptr) {}

bool MessageQueueMock::wasMessageSent() const {
  return std::any_of(
      sendMap.begin(), sendMap.end(),
      [](const std::pair<MessageQueueId_t, SendInfo>& pair) { return pair.second.callCount > 0; });
}

size_t MessageQueueMock::numberOfSentMessages() const {
  size_t callCount = 0;
  for (auto& destInfo : sendMap) {
    callCount += destInfo.second.callCount;
  }
  return callCount;
}

size_t MessageQueueMock::numberOfSentMessagesToDest(MessageQueueId_t id) const {
  auto iter = sendMap.find(id);
  if (iter == sendMap.end()) {
    return 0;
  }
  return iter->second.callCount;
}

size_t MessageQueueMock::numberOfSentMessagesToDefault() const {
  return numberOfSentMessagesToDest(MessageQueueBase::getDefaultDestination());
}

ReturnValue_t MessageQueueMock::clearLastReceivedMessage(bool clearCmdMsg) {
  if (receivedMsgs.empty()) {
    return MessageQueueIF::EMPTY;
  }
  if (clearCmdMsg) {
    CommandMessage message;
    std::memcpy(message.getBuffer(), receivedMsgs.front().getBuffer(), message.getMessageSize());
    message.clearCommandMessage();
  }
  receivedMsgs.pop();
  return returnvalue::OK;
}

ReturnValue_t MessageQueueMock::receiveMessage(MessageQueueMessageIF* message) {
  if (receivedMsgs.empty()) {
    return MessageQueueIF::EMPTY;
  }
  std::memcpy(message->getBuffer(), receivedMsgs.front().getBuffer(), message->getMessageSize());
  receivedMsgs.pop();
  return returnvalue::OK;
}

ReturnValue_t MessageQueueMock::flush(uint32_t* count) { return returnvalue::FAILED; }

ReturnValue_t MessageQueueMock::sendMessageFrom(MessageQueueId_t sendTo,
                                                MessageQueueMessageIF* message,
                                                MessageQueueId_t sentFrom, bool ignoreFault) {
  if (message == nullptr) {
    return returnvalue::FAILED;
  }
  if (nextSendFailsPair.first) {
    nextSendFailsPair.first = false;
    return nextSendFailsPair.second;
  }
  auto iter = sendMap.find(sendTo);
  MessageQueueMessage messageCopy;
  if (iter == sendMap.end()) {
    createMsgCopy(messageCopy, *message);
    sendMap.emplace(sendTo, SendInfo(messageCopy, 1));
  } else {
    iter->second.callCount += 1;
    createMsgCopy(messageCopy, *message);
    iter->second.msgs.push(messageCopy);
  }
  return returnvalue::OK;
}

ReturnValue_t MessageQueueMock::reply(MessageQueueMessageIF* message) {
  return sendMessageFrom(MessageQueueIF::NO_QUEUE, message, this->getId(), false);
}

void MessageQueueMock::clearMessages(bool clearCommandMessages) {
  if (not clearCommandMessages) {
    sendMap.clear();
    return;
  }
  for (auto& destInfo : sendMap) {
    while (!destInfo.second.msgs.empty()) {
      CommandMessage message;
      std::memcpy(message.getBuffer(), destInfo.second.msgs.front().getBuffer(),
                  message.getMessageSize());
      message.clear();
      destInfo.second.msgs.pop();
      destInfo.second.callCount--;
    }
  }
  sendMap.clear();
}

void MessageQueueMock::addReceivedMessage(MessageQueueMessageIF& msg) {
  MessageQueueMessage messageCopy;
  createMsgCopy(messageCopy, msg);
  receivedMsgs.push(messageCopy);
}

void MessageQueueMock::createMsgCopy(MessageQueueMessageIF& into, MessageQueueMessageIF& from) {
  if (from.getMessageSize() > into.getMaximumDataSize()) {
    throw std::invalid_argument("Passed message does not fit into message copy");
  }
  std::memcpy(into.getBuffer(), from.getBuffer(), from.getMaximumDataSize());
}

ReturnValue_t MessageQueueMock::getNextSentMessage(MessageQueueId_t id,
                                                   MessageQueueMessageIF& message) {
  auto iter = sendMap.find(id);
  if (iter == sendMap.end() or iter->second.callCount == 0) {
    return MessageQueueIF::EMPTY;
  }
  createMsgCopy(message, iter->second.msgs.front());
  return returnvalue::OK;
}

ReturnValue_t MessageQueueMock::getNextSentMessage(MessageQueueMessageIF& message) {
  return getNextSentMessage(MessageQueueBase::getDefaultDestination(), message);
}

ReturnValue_t MessageQueueMock::clearLastSentMessage(MessageQueueId_t destId, bool clearCmdMsg) {
  auto iter = sendMap.find(destId);
  if (iter == sendMap.end()) {
    return MessageQueueIF::EMPTY;
  }
  return clearLastSentMessage(iter, clearCmdMsg);
}

ReturnValue_t MessageQueueMock::clearLastSentMessage(bool clearCmdMsg) {
  auto iter = sendMap.find(getDefaultDestination());
  if (iter == sendMap.end()) {
    return MessageQueueIF::EMPTY;
  }
  ReturnValue_t result = clearLastSentMessage(iter, clearCmdMsg);
  clearEmptyEntries();
  return result;
}

ReturnValue_t MessageQueueMock::clearLastSentMessage(
    std::map<MessageQueueId_t, SendInfo>::iterator& iter, bool clearCmdMsg) {
  if (clearCmdMsg) {
    CommandMessage message;
    std::memcpy(message.getBuffer(), iter->second.msgs.front().getBuffer(),
                message.getMessageSize());
    message.clear();
  }
  iter->second.msgs.pop();
  iter->second.callCount--;
  return returnvalue::OK;
}
void MessageQueueMock::clearEmptyEntries() {
  for (auto it = sendMap.cbegin(); it != sendMap.cend();) {
    if (it->second.callCount == 0) {
      sendMap.erase(it++);
    } else {
      ++it;
    }
  }
}
void MessageQueueMock::makeNextSendFail(ReturnValue_t retval) {
  nextSendFailsPair.first = true;
  nextSendFailsPair.second = retval;
}

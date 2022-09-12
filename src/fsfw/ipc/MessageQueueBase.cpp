#include "MessageQueueBase.h"

MessageQueueBase::MessageQueueBase(MessageQueueId_t id, MessageQueueId_t defaultDest, MqArgs* args)
    : id(id) {
  this->defaultDest = defaultDest;
  if (args != nullptr) {
    this->args = *args;
  }
}

MessageQueueBase::~MessageQueueBase() = default;

ReturnValue_t MessageQueueBase::sendToDefault(MessageQueueMessageIF* message) {
  return sendToDefaultFrom(message, this->getId(), false);
}

ReturnValue_t MessageQueueBase::reply(MessageQueueMessageIF* message) {
  if (this->last != MessageQueueIF::NO_QUEUE) {
    return sendMessageFrom(this->last, message, this->getId());
  } else {
    return NO_REPLY_PARTNER;
  }
}

ReturnValue_t MessageQueueBase::receiveMessage(MessageQueueMessageIF* message,
                                               MessageQueueId_t* receivedFrom) {
  ReturnValue_t status = this->receiveMessage(message);
  *receivedFrom = this->last;
  return status;
}

MessageQueueId_t MessageQueueBase::getLastPartner() const { return last; }

MessageQueueId_t MessageQueueBase::getId() const { return id; }

MqArgs& MessageQueueBase::getMqArgs() { return args; }

void MessageQueueBase::setDefaultDestination(MessageQueueId_t defaultDestination) {
  this->defaultDest = defaultDestination;
}

MessageQueueId_t MessageQueueBase::getDefaultDestination() const { return defaultDest; }

bool MessageQueueBase::isDefaultDestinationSet() const { return (defaultDest != NO_QUEUE); }

ReturnValue_t MessageQueueBase::sendMessage(MessageQueueId_t sendTo, MessageQueueMessageIF* message,
                                            bool ignoreFault) {
  return sendMessageFrom(sendTo, message, this->getId(), false);
}

ReturnValue_t MessageQueueBase::sendToDefaultFrom(MessageQueueMessageIF* message,
                                                  MessageQueueId_t sentFrom, bool ignoreFault) {
  return sendMessageFrom(defaultDest, message, sentFrom, ignoreFault);
}

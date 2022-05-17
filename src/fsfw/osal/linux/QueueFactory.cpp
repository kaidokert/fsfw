#include "fsfw/ipc/QueueFactory.h"

#include <errno.h>
#include <mqueue.h>

#include <cstring>

#include "fsfw/ipc/MessageQueueSenderIF.h"
#include "fsfw/ipc/messageQueueDefinitions.h"
#include "fsfw/osal/linux/MessageQueue.h"

QueueFactory* QueueFactory::factoryInstance = nullptr;

ReturnValue_t MessageQueueSenderIF::sendMessage(MessageQueueId_t sendTo,
                                                MessageQueueMessageIF* message,
                                                MessageQueueId_t sentFrom, bool ignoreFault) {
  return MessageQueue::sendMessageFromMessageQueue(sendTo, message, sentFrom, ignoreFault);
}

QueueFactory* QueueFactory::instance() {
  if (factoryInstance == nullptr) {
    factoryInstance = new QueueFactory;
  }
  return factoryInstance;
}

QueueFactory::QueueFactory() {}

QueueFactory::~QueueFactory() {}

MessageQueueIF* QueueFactory::createMessageQueue(uint32_t messageDepth, size_t maxMessageSize,
                                                 MqArgs* args) {
  return new MessageQueue(messageDepth, maxMessageSize, args);
}

void QueueFactory::deleteMessageQueue(MessageQueueIF* queue) { delete queue; }

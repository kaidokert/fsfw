#include "fsfw/ipc/QueueFactory.h"

#include <cstring>

#include "fsfw/ipc/MessageQueueMessageIF.h"
#include "fsfw/ipc/MessageQueueSenderIF.h"
#include "fsfw/osal/host/MessageQueue.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

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

QueueFactory::QueueFactory() = default;

QueueFactory::~QueueFactory() = default;

MessageQueueIF* QueueFactory::createMessageQueue(uint32_t messageDepth, size_t maxMessageSize,
                                                 MqArgs* args) {
  // A thread-safe queue can be implemented  by using a combination
  // of std::queue and std::mutex. This uses dynamic memory allocation
  // which could be alleviated by using a custom allocator, external library
  // (etl::queue) or simply using std::queue, we're on a host machine anyway.
  return new MessageQueue(messageDepth, maxMessageSize, args);
}

void QueueFactory::deleteMessageQueue(MessageQueueIF* queue) { delete queue; }

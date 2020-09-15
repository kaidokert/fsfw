#include "../../ipc/MessageQueueSenderIF.h"
#include "../../ipc/QueueFactory.h"

#include "MessageQueue.h"


QueueFactory* QueueFactory::factoryInstance = NULL;


ReturnValue_t MessageQueueSenderIF::sendMessage(MessageQueueId_t sendTo,
			MessageQueueMessage* message, MessageQueueId_t sentFrom,
			bool ignoreFault) {
	return MessageQueue::sendMessageFromMessageQueue(sendTo, message,
			sentFrom, ignoreFault);
}

QueueFactory* QueueFactory::instance() {
	if (factoryInstance == NULL) {
		factoryInstance = new QueueFactory;
	}
	return factoryInstance;
}

QueueFactory::QueueFactory() {
}

QueueFactory::~QueueFactory() {
}

MessageQueueIF* QueueFactory::createMessageQueue(uint32_t message_depth,
		size_t maxMessageSize) {
	return new MessageQueue(message_depth, maxMessageSize);
}

void QueueFactory::deleteMessageQueue(MessageQueueIF* queue) {
	delete queue;
}

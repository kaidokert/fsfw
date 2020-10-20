#include "../../ipc/QueueFactory.h"
#include "MessageQueue.h"

#include "../../ipc/messageQueueDefinitions.h"
#include "../../ipc/MessageQueueSenderIF.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"

#include <mqueue.h>
#include <errno.h>


#include <cstring>

QueueFactory* QueueFactory::factoryInstance = nullptr;


ReturnValue_t MessageQueueSenderIF::sendMessage(MessageQueueId_t sendTo,
			MessageQueueMessageIF* message, MessageQueueId_t sentFrom,
			bool ignoreFault) {
	return MessageQueue::sendMessageFromMessageQueue(sendTo,message,
			sentFrom,ignoreFault);
}

QueueFactory* QueueFactory::instance() {
	if (factoryInstance == nullptr) {
		factoryInstance = new QueueFactory;
	}
	return factoryInstance;
}

QueueFactory::QueueFactory() {
}

QueueFactory::~QueueFactory() {
}

MessageQueueIF* QueueFactory::createMessageQueue(uint32_t messageDepth,
		size_t maxMessageSize) {
	return new MessageQueue(messageDepth, maxMessageSize);
}

void QueueFactory::deleteMessageQueue(MessageQueueIF* queue) {
	delete queue;
}

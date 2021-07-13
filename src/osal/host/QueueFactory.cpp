
#include "MessageQueue.h"

#include "../../ipc/MessageQueueSenderIF.h"
#include "../../ipc/MessageQueueMessageIF.h"
#include "../../ipc/QueueFactory.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"

#include <cstring>

QueueFactory* QueueFactory::factoryInstance = nullptr;


ReturnValue_t MessageQueueSenderIF::sendMessage(MessageQueueId_t sendTo,
			MessageQueueMessageIF* message, MessageQueueId_t sentFrom,
			bool ignoreFault) {
	return MessageQueue::sendMessageFromMessageQueue(sendTo,message,
			sentFrom,ignoreFault);
	return HasReturnvaluesIF::RETURN_OK;
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
	// A thread-safe queue can be implemented  by using a combination
	// of std::queue and std::mutex. This uses dynamic memory allocation
	// which could be alleviated by using a custom allocator, external library
	// (etl::queue) or simply using std::queue, we're on a host machine anyway.
	return new MessageQueue(messageDepth, maxMessageSize);
}

void QueueFactory::deleteMessageQueue(MessageQueueIF* queue) {
	delete queue;
}

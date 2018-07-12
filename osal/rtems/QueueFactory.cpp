#include <framework/ipc/QueueFactory.h>
#include "MessageQueue.h"
#include "RtemsBasic.h"

QueueFactory* QueueFactory::factoryInstance = NULL;


ReturnValue_t MessageQueueSenderIF::sendMessage(MessageQueueId_t sendTo,
			MessageQueueMessage* message, MessageQueueId_t sentFrom) {
	message->setSender(sentFrom);
	rtems_status_code result = rtems_message_queue_send(sendTo, message->getBuffer(),
			message->messageSize);
	return RtemsBasic::convertReturnCode(result);
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
		uint32_t max_message_size) {
	return new MessageQueue(message_depth, max_message_size);
}

void QueueFactory::deleteMessageQueue(MessageQueueIF* queue) {
	delete queue;
}

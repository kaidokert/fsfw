#include <framework/ipc/QueueFactory.h>

#include <framework/osal/FreeRTOS/MessageQueue.h>


QueueFactory* QueueFactory::factoryInstance = nullptr;


ReturnValue_t MessageQueueSenderIF::sendMessage(MessageQueueId_t sendTo,
			MessageQueueMessageIF* message, size_t maxSize,
			MessageQueueId_t sentFrom, bool ignoreFault) {
    if(maxSize == 0) {
        sif::error << "MessageQueueSenderIF::sendMessage: Max Size is 0!"
                << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
    }
	return MessageQueue::sendMessageFromMessageQueue(sendTo,message, maxSize,
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

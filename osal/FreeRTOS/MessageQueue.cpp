#include "MessageQueue.h"

#include <framework/serviceinterface/ServiceInterfaceStream.h>

// TODO I guess we should have a way of checking if we are in an ISR and then use the "fromISR" versions of all calls

MessageQueue::MessageQueue(size_t message_depth, size_t max_message_size) :
		lastPartner(0), defaultDestination(0) {
	handle = xQueueCreate(message_depth, max_message_size);
	if (handle == NULL) {
		//TODO
		;
	}
}

MessageQueue::~MessageQueue() {
	if (handle != NULL) {
		vQueueDelete(handle);
	}
}

ReturnValue_t MessageQueue::sendMessage(MessageQueueId_t sendTo,
		MessageQueueMessage* message, bool ignoreFault) {
	return sendMessageFrom(sendTo, message, this->getId(), ignoreFault);
}

ReturnValue_t MessageQueue::sendToDefault(MessageQueueMessage* message) {
	return sendToDefaultFrom(message, this->getId());
}

ReturnValue_t MessageQueue::reply(MessageQueueMessage* message) {
	if (this->lastPartner != 0) {
		return sendMessageFrom(this->lastPartner, message, this->getId());
	} else {
		//TODO: Good returnCode
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

ReturnValue_t MessageQueue::receiveMessage(MessageQueueMessage* message,
		MessageQueueId_t* receivedFrom) {
	ReturnValue_t status = this->receiveMessage(message);
	*receivedFrom = this->lastPartner;
	return status;
}

ReturnValue_t MessageQueue::receiveMessage(MessageQueueMessage* message) {
	BaseType_t result = xQueueReceive(handle, message, 0);
	if (result == pdPASS){
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return MessageQueueIF::EMPTY;
	}
}

MessageQueueId_t MessageQueue::getLastPartner() const {
	return lastPartner;
}

ReturnValue_t MessageQueue::flush(uint32_t* count) {
	//TODO FreeRTOS does not support flushing partially
	//Is always successful
	xQueueReset(handle);
	return HasReturnvaluesIF::RETURN_OK;
}

MessageQueueId_t MessageQueue::getId() const {
	return (MessageQueueId_t) handle;
}

void MessageQueue::setDefaultDestination(MessageQueueId_t defaultDestination) {
	this->defaultDestination = defaultDestination;
}

ReturnValue_t MessageQueue::sendMessageFrom(MessageQueueId_t sendTo,
		MessageQueueMessage* message, MessageQueueId_t sentFrom,
		bool ignoreFault) {
	message->setSender(sentFrom);

	BaseType_t result = xQueueSendToBack((void * )sendTo, message, 0);
	if (result != pdPASS) {
		if (!ignoreFault) {
			//TODO errr reporter
		}
		return MessageQueueIF::FULL;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t MessageQueue::sendToDefaultFrom(MessageQueueMessage* message,
		MessageQueueId_t sentFrom, bool ignoreFault) {
	return sendMessageFrom(defaultDestination,message,sentFrom,ignoreFault);
}

MessageQueueId_t MessageQueue::getDefaultDestination() const {
	return defaultDestination;
}

bool MessageQueue::isDefaultDestinationSet() const {
	return 0;
}


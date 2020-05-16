#include "MessageQueue.h"

#include <framework/serviceinterface/ServiceInterfaceStream.h>

// TODO I guess we should have a way of checking if we are in an ISR and then
// use the "fromISR" versions of all calls
// As a first step towards this, introduces system context variable which needs
// to be switched manually
// Haven't found function to find system context.
MessageQueue::MessageQueue(size_t messageDepth, size_t maxMessageSize) :
defaultDestination(0),lastPartner(0), callContext(CallContext::task)  {
	handle = xQueueCreate(messageDepth, maxMessageSize);
	if (handle == NULL) {
		sif::error << "MessageQueue creation failed" << std::endl;
	}
}

MessageQueue::~MessageQueue() {
	if (handle != NULL) {
		vQueueDelete(handle);
	}
}

void MessageQueue::switchSystemContext(CallContext callContext) {
	this->callContext = callContext;
}

ReturnValue_t MessageQueue::sendMessage(MessageQueueId_t sendTo,
		MessageQueueMessage* message, bool ignoreFault) {
	return sendMessageFrom(sendTo, message, this->getId(), ignoreFault);
}

ReturnValue_t MessageQueue::sendToDefault(MessageQueueMessage* message) {
	return sendToDefaultFrom(message, this->getId());
}

ReturnValue_t MessageQueue::sendToDefaultFrom(MessageQueueMessage* message,
		MessageQueueId_t sentFrom, bool ignoreFault) {
	return sendMessageFrom(defaultDestination,message,sentFrom,ignoreFault);
}

ReturnValue_t MessageQueue::reply(MessageQueueMessage* message) {
	if (this->lastPartner != 0) {
		return sendMessageFrom(this->lastPartner, message, this->getId());
	} else {
		return NO_REPLY_PARTNER;
	}
}

ReturnValue_t MessageQueue::sendMessageFrom(MessageQueueId_t sendTo,
		MessageQueueMessage* message, MessageQueueId_t sentFrom,
		bool ignoreFault) {
	return sendMessageFromMessageQueue(sendTo,message,sentFrom,ignoreFault, callContext);
}


ReturnValue_t MessageQueue::handleSendResult(BaseType_t result, bool ignoreFault) {
	if (result != pdPASS) {
		if (!ignoreFault) {
			InternalErrorReporterIF* internalErrorReporter = objectManager->get<InternalErrorReporterIF>(
					objects::INTERNAL_ERROR_REPORTER);
			if (internalErrorReporter != NULL) {
				internalErrorReporter->queueMessageNotSent();
			}
		}
		return MessageQueueIF::FULL;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t MessageQueue::receiveMessage(MessageQueueMessage* message,
		MessageQueueId_t* receivedFrom) {
	ReturnValue_t status = this->receiveMessage(message);
	if(status == HasReturnvaluesIF::RETURN_OK) {
		*receivedFrom = this->lastPartner;
	}
	return status;
}

ReturnValue_t MessageQueue::receiveMessage(MessageQueueMessage* message) {
	BaseType_t result = xQueueReceive(handle,reinterpret_cast<void*>(message->getBuffer()), 0);
	if (result == pdPASS){
		this->lastPartner = message->getSender();
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

MessageQueueId_t MessageQueue::getDefaultDestination() const {
	return defaultDestination;
}

bool MessageQueue::isDefaultDestinationSet() const {
	return 0;
}


// static core function to send messages.
ReturnValue_t MessageQueue::sendMessageFromMessageQueue(MessageQueueId_t sendTo,
        MessageQueueMessage *message, MessageQueueId_t sentFrom,
        bool ignoreFault, CallContext callContext) {
    message->setSender(sentFrom);
    BaseType_t result;
    if(callContext == CallContext::task) {
        result = xQueueSendToBack(reinterpret_cast<QueueHandle_t>(sendTo),
                static_cast<const void*>(message->getBuffer()), 0);
    }
    else {
        // If the call context is from an interrupt,
        // request a context switch if a higher priority task
        // was blocked by the interrupt.
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        result = xQueueSendFromISR(reinterpret_cast<QueueHandle_t>(sendTo),
                static_cast<const void*>(message->getBuffer()),
                &xHigherPriorityTaskWoken);
        if(xHigherPriorityTaskWoken == pdTRUE) {
            TaskManagement::requestContextSwitch(callContext);
        }
    }
    return handleSendResult(result, ignoreFault);
}

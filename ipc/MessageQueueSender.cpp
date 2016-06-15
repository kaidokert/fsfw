/*
 * MessageQueueSender.cpp
 *
 *  Created on: 22.11.2012
 *      Author: baetz
 */

#include <framework/ipc/MessageQueueMessage.h>
#include <framework/ipc/MessageQueueSender.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>


MessageQueueSender::MessageQueueSender(MessageQueueId_t set_default_destination) :
		default_destination(set_default_destination) {
	//Nothing to do in ctor.
}

void MessageQueueSender::setDefaultDestination(
		MessageQueueId_t defaultDestination) {
	default_destination = defaultDestination;
}

MessageQueueSender::~MessageQueueSender() {
	//Nothing to do in dtor.
}

ReturnValue_t MessageQueueSender::sendMessage(MessageQueueId_t sendTo,
		MessageQueueMessage* message, MessageQueueId_t sentFrom) {

	message->setSender(sentFrom);
	ReturnValue_t result = OSAL::sendMessage(sendTo, message->getBuffer(),
			message->messageSize);
	if (result != RETURN_OK) {
		debug << "MessageQueueSender. Sending message from " << std::hex
				<< sentFrom << " to " << sendTo << " failed with " << result
				<< std::endl;
	}
	return result;
}

ReturnValue_t MessageQueueSender::sendToDefault(MessageQueueMessage* message,
		MessageQueueId_t sentFrom) {
	message->setSender(sentFrom);
	ReturnValue_t result = OSAL::sendMessage(this->default_destination,
			message->getBuffer(), message->messageSize);
	if (result != RETURN_OK) {
		debug << "MessageQueueSender. Sending message from " << std::hex
				<< sentFrom << " to " << default_destination << " failed with "
				<< result << std::endl;
	}
	return result;
}

MessageQueueId_t MessageQueueSender::getDefaultDestination() {
	return this->default_destination;
}

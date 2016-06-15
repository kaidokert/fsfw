/*
 * MessageProxy.cpp
 *
 *  Created on: 18.03.2015
 *      Author: baetz
 */

#include <framework/ipc/MessageProxy.h>

MessageProxy::~MessageProxy() {
	
}

MessageQueueId_t MessageProxy::getReceiver() const {
	return receiver;
}

MessageQueueId_t MessageProxy::getCommandQueue() const {
	return commandQueue.getId();
}

void MessageProxy::setReceiver(MessageQueueId_t configuredReceiver) {
	this->receiver = configuredReceiver;
}

MessageProxy::MessageProxy(size_t queueDepth, MessageQueueId_t setReceiver) :
		receiver(setReceiver), currentRequest(0), commandQueue(queueDepth) {
}

void MessageProxy::checkCommandQueue() {
	CommandMessage message;
	MessageQueueId_t senderId;
	ReturnValue_t result = commandQueue.receiveMessage(&message, &senderId);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return;
	}
	if (senderId != receiver) {
		//It's a command.
		if (currentRequest == 0) {
			result = commandQueue.sendMessage(receiver, &message);
			if (result == HasReturnvaluesIF::RETURN_OK) {
				currentRequest = senderId;
			}
		} else {
			result = commandFifo.insert(message);
		}
		if (result != HasReturnvaluesIF::RETURN_OK) {
			message.clear();
			CommandMessage reply;
			reply.setReplyRejected(result, message.getCommand());
			commandQueue.reply(&reply);
		}
	} else {
		//It's a reply.
		if (currentRequest != 0) {
			//Failed forwarding is ignored.
			commandQueue.sendMessage(currentRequest, &message);
			//This request is finished.
			currentRequest = 0;
			//Check if there's another request in FIFO:
			result = commandFifo.retrieve(&message);
			if (result != HasReturnvaluesIF::RETURN_OK) {
				//Nothing in FIFO.
				return;
			}
			currentRequest = message.getSender();
			result = commandQueue.sendMessage(receiver, &message);
			if (result != HasReturnvaluesIF::RETURN_OK) {
				message.clear();
				CommandMessage reply;
				reply.setReplyRejected(result, message.getCommand());
				commandQueue.reply(&reply);
				currentRequest = 0;
			}
		} else {
			//We don't expect a reply. Ignore.
		}
	}
}

void MessageProxy::flush() {
	CommandMessage command;
	CommandMessage reply;
	if (currentRequest != 0) {
		commandQueue.sendMessage(currentRequest, &reply);
		currentRequest = 0;
	}
	for (ReturnValue_t result = commandQueue.receiveMessage(&command);
			result == HasReturnvaluesIF::RETURN_OK;
			result = commandQueue.receiveMessage(&command)) {
		reply.setReplyRejected(FLUSHED, command.getCommand());
		commandQueue.reply(&reply);
		command.clear();
	}
	for (ReturnValue_t result = commandFifo.retrieve(&command);
			result == HasReturnvaluesIF::RETURN_OK;
			result = commandFifo.retrieve(&command)) {
		reply.setReplyRejected(FLUSHED, command.getCommand());
		commandQueue.sendMessage(command.getSender(), &reply);
		command.clear();
	}
}

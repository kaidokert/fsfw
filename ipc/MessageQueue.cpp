/*
 * MessageQueue.cpp
 *
 *  Created on: Oct 2, 2012
 *      Author: baetz
 */

#include <framework/ipc/MessageQueue.h>
#include <framework/ipc/MessageQueueMessage.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>


MessageQueue::MessageQueue( size_t message_depth, size_t max_message_size ) :
		id(0), lastPartner(0) {
	Name_t name = ('Q' << 24) + (queueCounter++ << 8);
	ReturnValue_t status = OSAL::createMessageQueue(name, message_depth, max_message_size, 0, &(this->id));
	if (status != RETURN_OK) {
		error << "MessageQueue::MessageQueue: Creating Queue " << std::hex << name << std::dec << " failed with status:" << (uint32_t)status << std::endl;
		this->id = 0;
	} else {
		//Creating the MQ was successful
//		if (id == 0x220100f8) {
//			debug << "Queue found! " << std::endl;
//		}
	}
}

MessageQueue::~MessageQueue() {
	OSAL::deleteMessageQueue(&this->id);
}

ReturnValue_t MessageQueue::sendMessage(MessageQueueId_t sendTo,

		MessageQueueMessage* message) {
	return this->MessageQueueSender::sendMessage(sendTo, message, this->getId() );
}

ReturnValue_t MessageQueue::sendToDefault(MessageQueueMessage* message) {
	return this->MessageQueueSender::sendToDefault(message, this->getId() );
}

ReturnValue_t MessageQueue::reply(MessageQueueMessage* message) {
	if (this->lastPartner != 0) {
		return this->MessageQueueSender::sendMessage( this->lastPartner, message, this->getId() );
	} else {
		return OSAL::INCORRECT_STATE;
	}
}


ReturnValue_t MessageQueue::receiveMessage(MessageQueueMessage* message,
		MessageQueueId_t* receivedFrom) {
	ReturnValue_t status = this->receiveMessage(message);
	*receivedFrom = this->lastPartner;
	return status;
}

ReturnValue_t MessageQueue::receiveMessage(MessageQueueMessage* message) {
	ReturnValue_t status = OSAL::receiveMessage(this->id, message->getBuffer(), message->MAX_MESSAGE_SIZE,
			&(message->messageSize), OSAL::NO_WAIT, 1);
	if (status == RETURN_OK) {
		this->lastPartner = message->getSender();
		//Check size of incoming message.
		if ( message->messageSize < message->getMinimumMessageSize() ) {
			status = RETURN_FAILED;
		}
	} else {
		//No message was received. Keep lastPartner anyway, I might send something later.
		//But still, delete packet content.
		memset(message->getData(),0,message->MAX_DATA_SIZE);
	}
	return status;
}

MessageQueueId_t MessageQueue::getLastPartner() {
	return this->lastPartner;
}

ReturnValue_t MessageQueue::flush(uint32_t* count) {
	return OSAL::flushMessageQueue(this->id, count);
}

MessageQueueId_t MessageQueue::getId() const {
	return this->id;
}

uint16_t MessageQueue::queueCounter = 0;

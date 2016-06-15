/*
 * MessageQueueMessage.cpp
 *
 *  Created on: 22.11.2012
 *      Author: baetz
 */
#include <framework/ipc/MessageQueueMessage.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

MessageQueueMessage::MessageQueueMessage() :
		messageSize(this->HEADER_SIZE) {
	memset(this->internalBuffer, 0, sizeof(this->internalBuffer));
}

MessageQueueMessage::~MessageQueueMessage() {
}

const uint8_t* MessageQueueMessage::getBuffer() const {
	return this->internalBuffer;
}

uint8_t* MessageQueueMessage::getBuffer() {
	return this->internalBuffer;
}

const uint8_t* MessageQueueMessage::getData() const {
	return this->internalBuffer + this->HEADER_SIZE;
}

uint8_t* MessageQueueMessage::getData() {
	return this->internalBuffer + this->HEADER_SIZE;
}

MessageQueueId_t MessageQueueMessage::getSender() const {
	MessageQueueId_t temp_id;
	memcpy(&temp_id, this->internalBuffer, sizeof(MessageQueueId_t));
	return temp_id;
}

void MessageQueueMessage::setSender(MessageQueueId_t setId) {
	memcpy(this->internalBuffer, &setId, sizeof(MessageQueueId_t));
}

MessageQueueMessage::MessageQueueMessage(uint8_t* data, uint32_t size) :
		messageSize(this->HEADER_SIZE + size) {
	if (size <= this->MAX_DATA_SIZE) {
		memcpy(this->getData(), data, size);
	} else {
		memset(this->internalBuffer, 0, sizeof(this->internalBuffer));
		this->messageSize = this->HEADER_SIZE;
	}
}

size_t MessageQueueMessage::getMinimumMessageSize() {
	return this->HEADER_SIZE;
}

void MessageQueueMessage::print() {
	debug << "MessageQueueMessage has size: " << this->messageSize << std::hex
			<< std::endl;
	for (uint8_t count = 0; count < this->messageSize; count++) {
		debug << (uint32_t) this->internalBuffer[count] << ":";
	}
	debug << std::dec << std::endl;
}

void MessageQueueMessage::clear() {
	memset(this->getBuffer(), 0, this->MAX_MESSAGE_SIZE);
}

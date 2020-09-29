#include "MessageQueueMessage.h"
#include "../serviceinterface/ServiceInterfaceStream.h"
#include "../globalfunctions/arrayprinter.h"
#include <cstring>

MessageQueueMessage::MessageQueueMessage() :
		messageSize(getMinimumMessageSize()) {
	memset(this->internalBuffer, 0, sizeof(this->internalBuffer));
}

MessageQueueMessage::MessageQueueMessage(uint8_t* data, size_t size) :
		messageSize(this->HEADER_SIZE + size) {
	if (size <= this->MAX_DATA_SIZE) {
		memcpy(this->getData(), data, size);
		this->messageSize = this->HEADER_SIZE + size;
	}
	else {
		sif::warning << "MessageQueueMessage: Passed size larger than maximum"
				"allowed size! Setting content to 0" << std::endl;
		memset(this->internalBuffer, 0, sizeof(this->internalBuffer));
		this->messageSize = this->HEADER_SIZE;
	}
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

void MessageQueueMessage::print(bool printWholeMessage) {
	sif::debug << "MessageQueueMessage content: " << std::endl;
	if(printWholeMessage) {
	    arrayprinter::print(getData(), getMaximumMessageSize());
	}
	else {
	    arrayprinter::print(getData(), getMessageSize());
	}

}

void MessageQueueMessage::clear() {
	memset(this->getBuffer(), 0, this->MAX_MESSAGE_SIZE);
}

size_t MessageQueueMessage::getMessageSize() const {
	return this->messageSize;
}

void MessageQueueMessage::setMessageSize(size_t messageSize) {
	this->messageSize = messageSize;
}

size_t MessageQueueMessage::getMinimumMessageSize() const {
    return this->MIN_MESSAGE_SIZE;
}

size_t MessageQueueMessage::getMaximumMessageSize() const {
    return this->MAX_MESSAGE_SIZE;
}


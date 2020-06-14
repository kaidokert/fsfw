#include <framework/ipc/CommandMessageBase.h>
#include <framework/ipc/CommandMessageCleaner.h>
#include <cstring>

CommandMessageBase::CommandMessageBase(MessageQueueMessageIF *message):
		internalMessage(message) {
}

Command_t CommandMessageBase::getCommand() const {
	Command_t command;
	std::memcpy(&command, internalMessage->getData(), sizeof(Command_t));
	return command;
}

void CommandMessageBase::setCommand(Command_t command) {
	std::memcpy(internalMessage->getData(), &command, sizeof(Command_t));
}

uint8_t CommandMessageBase::getMessageType() const {
	// first byte of command ID.
	return getCommand() >> 8 & 0xff;
}

MessageQueueId_t CommandMessageBase::getSender() const {
	return internalMessage->getSender();
}

uint8_t* CommandMessageBase::getBuffer() {
	return internalMessage->getBuffer();
}

void CommandMessageBase::setSender(MessageQueueId_t setId) {
	internalMessage->setSender(setId);
}

const uint8_t* CommandMessageBase::getBuffer() const {
	return internalMessage->getBuffer();
}

// Header includes command ID.
uint8_t* CommandMessageBase::getData() {
	return internalMessage->getData() + sizeof(Command_t);
}

// Header includes command ID.
const uint8_t* CommandMessageBase::getData() const {
	return internalMessage->getData() + sizeof(Command_t);
}

void CommandMessageBase::setMessageSize(size_t messageSize) {
	internalMessage->setMessageSize(messageSize);
}

size_t CommandMessageBase::getMessageSize() const {
	return internalMessage->getMessageSize();
}

size_t CommandMessageBase::getMaximumMessageSize() const {
	return internalMessage->getMaximumMessageSize();
}

MessageQueueMessageIF* CommandMessageBase::getInternalMessage() const {
	return internalMessage;
}

size_t CommandMessageBase::getMinimumMessageSize() const {
	return MINIMUM_COMMAND_MESSAGE_BASE_SIZE;
}

void CommandMessageBase::setReplyRejected(ReturnValue_t reason,
		Command_t initialCommand) {
	std::memcpy(getData(), &reason, sizeof(reason));
	std::memcpy(getData() + sizeof(reason), &initialCommand,
			sizeof(initialCommand));
}

ReturnValue_t CommandMessageBase::getReplyRejectedReason(
		Command_t *initialCommand) const {
	ReturnValue_t reason = HasReturnvaluesIF::RETURN_FAILED;
	std::memcpy(&reason, getData(), sizeof(reason));
	if(initialCommand != nullptr) {
		std::memcpy(initialCommand, getData() + sizeof(reason),
				sizeof(Command_t));
	}
	return reason;
}

void CommandMessageBase::clear() {
	CommandMessageCleaner::clearCommandMessage(this);
}

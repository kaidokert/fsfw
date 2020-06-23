#include <framework/ipc/CommandMessage.h>
#include <framework/ipc/CommandMessageCleaner.h>
#include <cstring>

CommandMessage::CommandMessage() {
	MessageQueueMessage::setMessageSize(DEFAULT_COMMAND_MESSAGE_SIZE);
    setCommand(CMD_NONE);
}

CommandMessage::CommandMessage(Command_t command, uint32_t parameter1,
        uint32_t parameter2) {
    MessageQueueMessage::setMessageSize(DEFAULT_COMMAND_MESSAGE_SIZE);
	setCommand(command);
	setParameter(parameter1);
	setParameter2(parameter2);
}

Command_t CommandMessage::getCommand() const {
  Command_t command;
  std::memcpy(&command, getData(), sizeof(Command_t));
  return command;
}

void CommandMessage::setCommand(Command_t command) {
  std::memcpy(getData(), &command, sizeof(Command_t));
}

uint8_t CommandMessage::getMessageType() const {
  // first byte of command ID.
  return getCommand() >> 8 & 0xff;
}

uint32_t CommandMessage::getParameter() const {
	uint32_t parameter1;
	std::memcpy(&parameter1, MessageQueueMessage::getData(), sizeof(parameter1));
	return parameter1;
}

void CommandMessage::setParameter(uint32_t parameter1) {
	std::memcpy(MessageQueueMessage::getData(), &parameter1, sizeof(parameter1));
}

uint32_t CommandMessage::getParameter2() const {
	uint32_t parameter2;
	std::memcpy(&parameter2, MessageQueueMessage::getData() + sizeof(uint32_t),
			sizeof(parameter2));
	return parameter2;
}

void CommandMessage::setParameter2(uint32_t parameter2) {
	std::memcpy(MessageQueueMessage::getData()  + sizeof(uint32_t), &parameter2,
			sizeof(parameter2));
}

size_t CommandMessage::getMinimumMessageSize() const {
	return MINIMUM_COMMAND_MESSAGE_SIZE;
}

void CommandMessage::clear() {
    CommandMessageCleaner::clearCommandMessage(this);
}

bool CommandMessage::isClearedCommandMessage() {
	return getCommand() == CMD_NONE;
}

void CommandMessage::setToUnknownCommand() {
	Command_t initialCommand = getCommand();
	this->clear();
	setReplyRejected(UNKNOWN_COMMAND, initialCommand);
}

void CommandMessage::setReplyRejected(ReturnValue_t reason,
      Command_t initialCommand) {
  std::memcpy(getData(), &reason, sizeof(reason));
  std::memcpy(getData() + sizeof(reason), &initialCommand,
          sizeof(initialCommand));
}

ReturnValue_t CommandMessage::getReplyRejectedReason(
      Command_t *initialCommand) const {
  ReturnValue_t reason = HasReturnvaluesIF::RETURN_FAILED;
  std::memcpy(&reason, getData(), sizeof(reason));
  if(initialCommand != nullptr) {
      std::memcpy(initialCommand, getData() + sizeof(reason),
              sizeof(Command_t));
  }
  return reason;
}

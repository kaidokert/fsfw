#include "fsfw/ipc/CommandMessage.h"

#include <cstring>

#include "fsfw/ipc/CommandMessageCleaner.h"

CommandMessage::CommandMessage() {
  MessageQueueMessage::setMessageSize(DEFAULT_COMMAND_MESSAGE_SIZE);
  setCommand(CMD_NONE);
}

CommandMessage::CommandMessage(Command_t command, uint32_t parameter1, uint32_t parameter2) {
  MessageQueueMessage::setMessageSize(DEFAULT_COMMAND_MESSAGE_SIZE);
  setCommand(command);
  setParameter(parameter1);
  setParameter2(parameter2);
}

Command_t CommandMessage::getCommand() const {
  Command_t command;
  std::memcpy(&command, MessageQueueMessage::getData(), sizeof(Command_t));
  return command;
}

void CommandMessage::setCommand(Command_t command) {
  std::memcpy(MessageQueueMessage::getData(), &command, sizeof(Command_t));
}

uint8_t CommandMessage::getMessageType() const {
  // first byte of command ID.
  return getCommand() >> 8 & 0xff;
}

uint32_t CommandMessage::getParameter() const {
  uint32_t parameter1;
  std::memcpy(&parameter1, this->getData(), sizeof(parameter1));
  return parameter1;
}

void CommandMessage::setParameter(uint32_t parameter1) {
  std::memcpy(this->getData(), &parameter1, sizeof(parameter1));
}

uint32_t CommandMessage::getParameter2() const {
  uint32_t parameter2;
  std::memcpy(&parameter2, this->getData() + sizeof(uint32_t), sizeof(parameter2));
  return parameter2;
}

void CommandMessage::setParameter2(uint32_t parameter2) {
  std::memcpy(this->getData() + sizeof(uint32_t), &parameter2, sizeof(parameter2));
}

uint32_t CommandMessage::getParameter3() const {
  uint32_t parameter3;
  std::memcpy(&parameter3, this->getData() + 2 * sizeof(uint32_t), sizeof(parameter3));
  return parameter3;
}

void CommandMessage::setParameter3(uint32_t parameter3) {
  std::memcpy(this->getData() + 2 * sizeof(uint32_t), &parameter3, sizeof(parameter3));
}

size_t CommandMessage::getMinimumMessageSize() const { return MINIMUM_COMMAND_MESSAGE_SIZE; }

void CommandMessage::clearCommandMessage() { clear(); }

void CommandMessage::clear() { CommandMessageCleaner::clearCommandMessage(this); }

bool CommandMessage::isClearedCommandMessage() { return getCommand() == CMD_NONE; }

void CommandMessage::setToUnknownCommand() {
  Command_t initialCommand = getCommand();
  this->clear();
  setReplyRejected(UNKNOWN_COMMAND, initialCommand);
}

void CommandMessage::setReplyRejected(ReturnValue_t reason, Command_t initialCommand) {
  setCommand(REPLY_REJECTED);
  setParameter(reason);
  setParameter2(initialCommand);
}

ReturnValue_t CommandMessage::getReplyRejectedReason(Command_t* initialCommand) const {
  ReturnValue_t reason = getParameter();
  if (initialCommand != nullptr) {
    *initialCommand = getParameter2();
  }
  return reason;
}

uint8_t* CommandMessage::getData() { return MessageQueueMessage::getData() + sizeof(Command_t); }

const uint8_t* CommandMessage::getData() const {
  return MessageQueueMessage::getData() + sizeof(Command_t);
}

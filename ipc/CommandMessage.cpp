/**
 * @file	CommandMessage.cpp
 * @brief	This file defines the CommandMessage class.
 * @date	20.06.2013
 * @author	baetz
 */

#include "../devicehandlers/DeviceHandlerMessage.h"
#include "../health/HealthMessage.h"
#include "CommandMessage.h"
#include "../memory/MemoryMessage.h"
#include "../modes/ModeMessage.h"
#include "../monitoring/MonitoringMessage.h"
#include "../subsystem/modes/ModeSequenceMessage.h"
#include "../tmstorage/TmStoreMessage.h"
#include "../parameters/ParameterMessage.h"

namespace messagetypes {
void clearMissionMessage(CommandMessage* message);
}


CommandMessage::CommandMessage() {
	this->messageSize = COMMAND_MESSAGE_SIZE;
	setCommand(CMD_NONE);
}

CommandMessage::CommandMessage(Command_t command, uint32_t parameter1,
		uint32_t parameter2) {
	this->messageSize = COMMAND_MESSAGE_SIZE;
	setCommand(command);
	setParameter(parameter1);
	setParameter2(parameter2);
}

Command_t CommandMessage::getCommand() const {
	Command_t command;
	memcpy(&command, getData(), sizeof(Command_t));
	return command;
}

void CommandMessage::setCommand(Command_t command) {
	memcpy(getData(), &command, sizeof(command));
}

uint32_t CommandMessage::getParameter() const {
	uint32_t parameter1;
	memcpy(&parameter1, getData() + sizeof(Command_t), sizeof(parameter1));
	return parameter1;
}

void CommandMessage::setParameter(uint32_t parameter1) {
	memcpy(getData() + sizeof(Command_t), &parameter1, sizeof(parameter1));
}

uint32_t CommandMessage::getParameter2() const {
	uint32_t parameter2;
	memcpy(&parameter2, getData() + sizeof(Command_t) + sizeof(uint32_t),
			sizeof(parameter2));
	return parameter2;
}

void CommandMessage::setParameter2(uint32_t parameter2) {
	memcpy(getData() + sizeof(Command_t) + sizeof(uint32_t), &parameter2,
			sizeof(parameter2));
}

void CommandMessage::clearCommandMessage() {
	switch((getCommand()>>8) & 0xff){
	case  messagetypes::MODE_COMMAND:
		ModeMessage::clear(this);
		break;
	case messagetypes::HEALTH_COMMAND:
		HealthMessage::clear(this);
		break;
	case messagetypes::MODE_SEQUENCE:
		ModeSequenceMessage::clear(this);
		break;
	case messagetypes::ACTION:
		ActionMessage::clear(this);
		break;
	case messagetypes::DEVICE_HANDLER_COMMAND:
		DeviceHandlerMessage::clear(this);
		break;
	case messagetypes::MEMORY:
		MemoryMessage::clear(this);
		break;
	case messagetypes::MONITORING:
		MonitoringMessage::clear(this);
		break;
	case messagetypes::TM_STORE:
		TmStoreMessage::clear(this);
		break;
	case messagetypes::PARAMETER:
		ParameterMessage::clear(this);
		break;
	default:
		messagetypes::clearMissionMessage(this);
		break;
	}
}

bool CommandMessage::isClearedCommandMessage() {
	return getCommand() == CMD_NONE;
}

size_t CommandMessage::getMinimumMessageSize() const {
	return COMMAND_MESSAGE_SIZE;
}

void CommandMessage::setToUnknownCommand() {
	Command_t initialCommand = getCommand();
	clearCommandMessage();
	setReplyRejected(UNKNOWN_COMMAND, initialCommand);
}

void CommandMessage::setReplyRejected(ReturnValue_t reason,
		Command_t initialCommand) {
	setCommand(REPLY_REJECTED);
	setParameter(reason);
	setParameter2(initialCommand);
}

ReturnValue_t CommandMessage::getReplyRejectedReason(
        Command_t *initialCommand) const {
    ReturnValue_t reason = getParameter();
    if(initialCommand != nullptr) {
        *initialCommand = getParameter2();
    }
    return reason;
}

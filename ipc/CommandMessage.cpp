#include <framework/ipc/CommandMessage.h>

#include <framework/devicehandlers/DeviceHandlerMessage.h>
#include <framework/health/HealthMessage.h>
#include <framework/memory/MemoryMessage.h>
#include <framework/modes/ModeMessage.h>
#include <framework/monitoring/MonitoringMessage.h>
#include <framework/subsystem/modes/ModeSequenceMessage.h>
#include <framework/tmstorage/TmStoreMessage.h>
#include <framework/parameters/ParameterMessage.h>

CommandMessage::CommandMessage(MessageQueueMessageIF* receiverMessage):
			CommandMessageBase(receiverMessage) {
	if(receiverMessage == nullptr) {
		sif::error << "CommandMessage::CommandMessage: Don't pass a nullptr"
				" as the message queue message, pass the address of an actual"
				" message!" << std::endl;
		return;
	}
	if(receiverMessage->getMaximumMessageSize() <
			MINIMUM_COMMAND_MESSAGE_SIZE) {
		sif::error << "CommandMessage::ComandMessage: Passed message buffer"
				" can not hold minimum "<< MINIMUM_COMMAND_MESSAGE_SIZE
				<< " bytes!" << std::endl;
		return;
	}
	internalMessage->setMessageSize(MINIMUM_COMMAND_MESSAGE_SIZE);
}

CommandMessage::CommandMessage(MessageQueueMessageIF* messageToSet,
		Command_t command, uint32_t parameter1, uint32_t parameter2):
				CommandMessageBase(messageToSet) {
	if(messageToSet == nullptr) {
		sif::error << "CommandMessage::CommandMessage: Don't pass a nullptr"
				" as the message queue message, pass the address of an actual"
				" message!" << std::endl;
	}
	if(messageToSet->getMaximumMessageSize() <
			MINIMUM_COMMAND_MESSAGE_SIZE) {
		sif::error << "CommandMessage::ComandMessage: Passed message buffer"
				" can not hold minimum "<< MINIMUM_COMMAND_MESSAGE_SIZE
				<< " bytes!" << std::endl;
		return;
	}
	internalMessage->setMessageSize(MINIMUM_COMMAND_MESSAGE_SIZE);
	setCommand(command);
	setParameter(parameter1);
	setParameter2(parameter2);
}

uint32_t CommandMessage::getParameter() const {
	uint32_t parameter1;
	memcpy(&parameter1, CommandMessageBase::getData(), sizeof(parameter1));
	return parameter1;
}

void CommandMessage::setParameter(uint32_t parameter1) {
	memcpy(CommandMessageBase::getData(), &parameter1, sizeof(parameter1));
}

uint32_t CommandMessage::getParameter2() const {
	uint32_t parameter2;
	memcpy(&parameter2, CommandMessageBase::getData() + sizeof(uint32_t),
			sizeof(parameter2));
	return parameter2;
}

void CommandMessage::setParameter2(uint32_t parameter2) {
	memcpy(CommandMessageBase::getData()  + sizeof(uint32_t), &parameter2,
			sizeof(parameter2));
}

size_t CommandMessage::getMinimumMessageSize() const {
	return MINIMUM_COMMAND_MESSAGE_SIZE;
}

size_t CommandMessage::getMaximumMessageSize() const {
	return MessageQueueMessage::MAX_MESSAGE_SIZE;
}

bool CommandMessage::isClearedCommandMessage() {
	return getCommand() == CMD_NONE;
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

ReturnValue_t CommandMessage::getRejectedReplyReason(
		Command_t* initialCommand) const {
	if(initialCommand != nullptr) {
		*initialCommand  = getParameter2();
	}
	return getParameter();
}

void CommandMessage::clear() {
	clearCommandMessage();
}

void CommandMessage::clearCommandMessage() {
	switch(this->getMessageType()){
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

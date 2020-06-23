#include <framework/ipc/CommandMessage.h>
#include <cstring>

CommandMessage::CommandMessage(MessageQueueMessageIF* receiverMessage):
			CommandMessageBase(receiverMessage) {
	if(receiverMessage == nullptr) {
		sif::error << "CommandMessage::CommandMessage: Don't pass a nullptr"
				" as the message queue message, pass the address of an actual"
				" message!" << std::endl;
		return;
	}
//	if(receiverMessage->getMessageSize() <
//			getMinimumMessageSize()) {
//		sif::error << "CommandMessage::ComandMessage: Passed message buffer"
//				" can not hold minimum "<< getMinimumMessageSize()
//				<< " bytes!" << std::endl;
//		return;
//	}
//	internalMessage->setMessageSize(getMinimumMessageSize());
}

CommandMessage::CommandMessage(MessageQueueMessageIF* messageToSet,
		Command_t command, uint32_t parameter1, uint32_t parameter2):
		CommandMessage(messageToSet) {
	setCommand(command);
	setParameter(parameter1);
	setParameter2(parameter2);
}

uint32_t CommandMessage::getParameter() const {
	uint32_t parameter1;
	std::memcpy(&parameter1, CommandMessageBase::getData(), sizeof(parameter1));
	return parameter1;
}

void CommandMessage::setParameter(uint32_t parameter1) {
	std::memcpy(CommandMessageBase::getData(), &parameter1, sizeof(parameter1));
}

uint32_t CommandMessage::getParameter2() const {
	uint32_t parameter2;
	std::memcpy(&parameter2, CommandMessageBase::getData() + sizeof(uint32_t),
			sizeof(parameter2));
	return parameter2;
}

void CommandMessage::setParameter2(uint32_t parameter2) {
	std::memcpy(CommandMessageBase::getData()  + sizeof(uint32_t), &parameter2,
			sizeof(parameter2));
}

//size_t CommandMessage::getMinimumMessageSize() const {
//	return MINIMUM_COMMAND_MESSAGE_SIZE;
//}

bool CommandMessage::isClearedCommandMessage() {
	return getCommand() == CMD_NONE;
}

void CommandMessage::setToUnknownCommand() {
	Command_t initialCommand = getCommand();
	this->clear();
	setReplyRejected(UNKNOWN_COMMAND, initialCommand);
}

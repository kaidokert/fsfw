/*
 * ModeMessage.cpp
 *
 *  Created on: 12.07.2013
 *      Author: tod
 */

#include <framework/modes/ModeMessage.h>

Mode_t ModeMessage::getMode(const CommandMessage* message) {
	return message->getParameter();
}

Submode_t ModeMessage::getSubmode(const CommandMessage* message) {
	return message->getParameter2();
}

ReturnValue_t ModeMessage::setModeMessage(CommandMessage* message, Command_t command,
		Mode_t mode, Submode_t submode) {
	message->setCommand( command );
	message->setParameter( mode );
	message->setParameter2( submode );
	return HasReturnvaluesIF::RETURN_OK;
}

void ModeMessage::clear(CommandMessage* message) {
	message->setCommand(CommandMessage::CMD_NONE);
}

void ModeMessage::cantReachMode(CommandMessage* message, ReturnValue_t reason) {
	message->setCommand(REPLY_CANT_REACH_MODE);
	message->setParameter(reason);
	message->setParameter2(0);
}

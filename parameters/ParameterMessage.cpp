/*
 * ParameterMessage.cpp
 *
 *  Created on: 28.11.2015
 *      Author: mohr
 */

#include <framework/parameters/ParameterMessage.h>

ParameterId_t ParameterMessage::getParameterId(const CommandMessage* message) {
	return message->getParameter();
}

store_address_t ParameterMessage::getStoreId(const CommandMessage* message) {
	store_address_t address;
	address.raw = message->getParameter2();
	return address;
}

void ParameterMessage::setParameterDumpCommand(CommandMessage* message,
		ParameterId_t id) {
	message->setCommand(CMD_PARAMETER_DUMP);
	message->setParameter(id);
}

void ParameterMessage::setParameterDumpReply(CommandMessage* message,
		ParameterId_t id, store_address_t storageID) {
	message->setCommand(REPLY_PARAMETER_DUMP);
	message->setParameter(id);
	message->setParameter2(storageID.raw);
}

void ParameterMessage::setParameterLoadCommand(CommandMessage* message,
		ParameterId_t id, store_address_t storageID) {
	message->setCommand(CMD_PARAMETER_LOAD);
	message->setParameter(id);
	message->setParameter2(storageID.raw);
}

/*
 * FileSystemMessage.cpp
 *
 *  Created on: 19.01.2020
 *      Author: Jakob Meier
 */

#include "FileSystemMessage.h"
#include "../objectmanager/ObjectManagerIF.h"

ReturnValue_t FileSystemMessage::setWriteToFileCommand(CommandMessage* message,
		MessageQueueId_t replyQueueId, store_address_t storageID) {
	message->setCommand(WRITE_TO_FILE);
	message->setParameter(replyQueueId);
	message->setParameter2(storageID.raw);
	return HasReturnvaluesIF::RETURN_OK;
}

store_address_t FileSystemMessage::getStoreID(const CommandMessage* message) {
	store_address_t temp;
	temp.raw = message->getParameter2();
	return temp;
}

MessageQueueId_t FileSystemMessage::getReplyQueueId(const CommandMessage* message){
	return message->getParameter();
}


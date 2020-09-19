#include "FileSystemMessage.h"
#include "../objectmanager/ObjectManagerIF.h"


ReturnValue_t FileSystemMessage::setDeleteFileCommand(
		CommandMessage* message, store_address_t storageID) {
	message->setCommand(DELETE_FILE);
	message->setParameter2(storageID.raw);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t FileSystemMessage::setCreateDirectoryCommand(
		CommandMessage* message, store_address_t storageID) {
	message->setCommand(CREATE_DIRECTORY);
	message->setParameter2(storageID.raw);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t FileSystemMessage::setDeleteDirectoryCommand(
		CommandMessage* message, store_address_t storageID) {
	message->setCommand(DELETE_DIRECTORY);
	message->setParameter2(storageID.raw);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t FileSystemMessage::setWriteCommand(CommandMessage* message,
		store_address_t storageID) {
	message->setCommand(WRITE);
	message->setParameter2(storageID.raw);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t FileSystemMessage::setReadCommand(CommandMessage* message,
		store_address_t storageID) {
	message->setCommand(READ);
	message->setParameter2(storageID.raw);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t FileSystemMessage::setReadReply(CommandMessage* message,
		store_address_t storageID) {
	message->setCommand(READ_REPLY);
	message->setParameter2(storageID.raw);
	return HasReturnvaluesIF::RETURN_OK;
}

store_address_t FileSystemMessage::getStoreId(const CommandMessage* message) {
	store_address_t temp;
	temp.raw = message->getParameter2();
	return temp;
}

void FileSystemMessage::setCompletionReply(CommandMessage* message,
		Command_t completionStatus) {
	message->setCommand(completionStatus);
}


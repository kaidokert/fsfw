#include "FileSystemMessage.h"
#include "../objectmanager/ObjectManagerIF.h"

void FileSystemMessage::setCreateFileCommand(CommandMessage* message,
        store_address_t storeId) {
    message->setCommand(CREATE_FILE);
    message->setParameter2(storeId.raw);
}

void FileSystemMessage::setDeleteFileCommand(
        CommandMessage* message, store_address_t storeId) {
    message->setCommand(DELETE_FILE);
    message->setParameter2(storeId.raw);
}

void FileSystemMessage::setCreateDirectoryCommand(
		CommandMessage* message, store_address_t storeId) {
	message->setCommand(CREATE_DIRECTORY);
	message->setParameter2(storeId.raw);
}

void FileSystemMessage::setDeleteDirectoryCommand(
		CommandMessage* message, store_address_t storeId) {
	message->setCommand(DELETE_DIRECTORY);
	message->setParameter2(storeId.raw);
}

void FileSystemMessage::setWriteCommand(CommandMessage* message,
		store_address_t storeId) {
	message->setCommand(APPEND_TO_FILE);
	message->setParameter2(storeId.raw);
}

void FileSystemMessage::setReadCommand(CommandMessage* message,
		store_address_t storeId) {
	message->setCommand(READ_FROM_FILE);
	message->setParameter2(storeId.raw);
}

void FileSystemMessage::setReadReply(CommandMessage* message,
		store_address_t storageID) {
	message->setCommand(READ_REPLY);
	message->setParameter2(storageID.raw);
}

store_address_t FileSystemMessage::getStoreId(const CommandMessage* message) {
	store_address_t temp;
	temp.raw = message->getParameter2();
	return temp;
}

void FileSystemMessage::setSuccessReply(CommandMessage *message) {
    message->setCommand(COMPLETION_SUCCESS);
}

void FileSystemMessage::setFailureReply(CommandMessage *message,
        ReturnValue_t errorCode) {
    message->setCommand(COMPLETION_FAILED);
    message->setParameter(errorCode);
}

ReturnValue_t FileSystemMessage::getFailureReply(
		const CommandMessage *message) {
	return message->getParameter();
}


#include "GenericFileSystemMessage.h"


void GenericFileSystemMessage::setCreateFileCommand(CommandMessage* message,
        store_address_t storeId) {
    message->setCommand(CMD_CREATE_FILE);
    message->setParameter2(storeId.raw);
}

void GenericFileSystemMessage::setDeleteFileCommand(
        CommandMessage* message, store_address_t storeId) {
    message->setCommand(CMD_DELETE_FILE);
    message->setParameter2(storeId.raw);
}

void GenericFileSystemMessage::setCreateDirectoryCommand(
        CommandMessage* message, store_address_t storeId) {
    message->setCommand(CMD_CREATE_DIRECTORY);
    message->setParameter2(storeId.raw);
}

void GenericFileSystemMessage::setReportFileAttributesCommand(CommandMessage *message,
        store_address_t storeId) {
    message->setCommand(CMD_REPORT_FILE_ATTRIBUTES);
    message->setParameter2(storeId.raw);
}

void GenericFileSystemMessage::setReportFileAttributesReply(CommandMessage *message,
        store_address_t storeId) {
    message->setCommand(REPLY_REPORT_FILE_ATTRIBUTES);
    message->setParameter2(storeId.raw);
}

void GenericFileSystemMessage::setDeleteDirectoryCommand(CommandMessage* message,
        store_address_t storeId) {
    message->setCommand(CMD_DELETE_DIRECTORY);
    message->setParameter2(storeId.raw);
}

void GenericFileSystemMessage::setLockFileCommand(CommandMessage *message,
        store_address_t storeId) {
    message->setCommand(CMD_LOCK_FILE);
    message->setParameter2(storeId.raw);
}

void GenericFileSystemMessage::setUnlockFileCommand(CommandMessage *message,
        store_address_t storeId) {
    message->setCommand(CMD_UNLOCK_FILE);
    message->setParameter2(storeId.raw);
}

void GenericFileSystemMessage::setSuccessReply(CommandMessage *message) {
    message->setCommand(COMPLETION_SUCCESS);
}

void GenericFileSystemMessage::setFailureReply(CommandMessage *message,
        ReturnValue_t errorCode, uint32_t errorParam) {
    message->setCommand(COMPLETION_FAILED);
    message->setParameter(errorCode);
    message->setParameter2(errorParam);
}


#include "GenericFileSystemMessage.h"

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/storagemanager/StorageManagerIF.h"

void GenericFileSystemMessage::setCreateFileCommand(CommandMessage *message,
                                                    store_address_t storeId) {
  message->setCommand(CMD_CREATE_FILE);
  message->setParameter2(storeId.raw);
}

void GenericFileSystemMessage::setDeleteFileCommand(CommandMessage *message,
                                                    store_address_t storeId) {
  message->setCommand(CMD_DELETE_FILE);
  message->setParameter2(storeId.raw);
}

void GenericFileSystemMessage::setCreateDirectoryCommand(CommandMessage *message,
                                                         store_address_t storeId) {
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

void GenericFileSystemMessage::setDeleteDirectoryCommand(CommandMessage *message,
                                                         store_address_t storeId,
                                                         bool deleteRecursively) {
  message->setCommand(CMD_DELETE_DIRECTORY);
  message->setParameter(deleteRecursively);
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

void GenericFileSystemMessage::setFailureReply(CommandMessage *message, ReturnValue_t errorCode,
                                               uint32_t errorParam) {
  message->setCommand(COMPLETION_FAILED);
  message->setParameter(errorCode);
  message->setParameter2(errorParam);
}

store_address_t GenericFileSystemMessage::getStoreId(const CommandMessage *message) {
  store_address_t temp;
  temp.raw = message->getParameter2();
  return temp;
}

ReturnValue_t GenericFileSystemMessage::getFailureReply(const CommandMessage *message,
                                                        uint32_t *errorParam) {
  if (errorParam != nullptr) {
    *errorParam = message->getParameter2();
  }
  return message->getParameter();
}

void GenericFileSystemMessage::setFinishStopWriteCommand(CommandMessage *message,
                                                         store_address_t storeId) {
  message->setCommand(CMD_FINISH_APPEND_TO_FILE);
  message->setParameter2(storeId.raw);
}

void GenericFileSystemMessage::setFinishStopWriteReply(CommandMessage *message,
                                                       store_address_t storeId) {
  message->setCommand(REPLY_FINISH_APPEND);
  message->setParameter2(storeId.raw);
}

void GenericFileSystemMessage::setCopyCommand(CommandMessage *message, store_address_t storeId) {
  message->setCommand(CMD_COPY_FILE);
  message->setParameter2(storeId.raw);
}

void GenericFileSystemMessage::setWriteCommand(CommandMessage *message, store_address_t storeId) {
  message->setCommand(CMD_APPEND_TO_FILE);
  message->setParameter2(storeId.raw);
}

void GenericFileSystemMessage::setReadCommand(CommandMessage *message, store_address_t storeId) {
  message->setCommand(CMD_READ_FROM_FILE);
  message->setParameter2(storeId.raw);
}

void GenericFileSystemMessage::setFinishAppendReply(CommandMessage *message,
                                                    store_address_t storageID) {
  message->setCommand(REPLY_FINISH_APPEND);
  message->setParameter2(storageID.raw);
}

void GenericFileSystemMessage::setReadReply(CommandMessage *message, bool readFinished,
                                            store_address_t storeId) {
  message->setCommand(REPLY_READ_FROM_FILE);
  message->setParameter(readFinished);
  message->setParameter2(storeId.raw);
}

void GenericFileSystemMessage::setReadFinishedReply(CommandMessage *message,
                                                    store_address_t storeId) {
  message->setCommand(REPLY_READ_FINISHED_STOP);
  message->setParameter2(storeId.raw);
}

bool GenericFileSystemMessage::getReadReply(const CommandMessage *message,
                                            store_address_t *storeId) {
  if (storeId != nullptr) {
    (*storeId).raw = message->getParameter2();
  }
  return message->getParameter();
}

store_address_t GenericFileSystemMessage::getDeleteDirectoryCommand(const CommandMessage *message,
                                                                    bool &deleteRecursively) {
  deleteRecursively = message->getParameter();
  return getStoreId(message);
}

ReturnValue_t GenericFileSystemMessage::clear(CommandMessage *message) {
  switch (message->getCommand()) {
    case (CMD_CREATE_FILE):
    case (CMD_DELETE_FILE):
    case (CMD_CREATE_DIRECTORY):
    case (CMD_REPORT_FILE_ATTRIBUTES):
    case (REPLY_REPORT_FILE_ATTRIBUTES):
    case (CMD_LOCK_FILE):
    case (CMD_UNLOCK_FILE):
    case (CMD_COPY_FILE):
    case (REPLY_READ_FROM_FILE):
    case (CMD_READ_FROM_FILE):
    case (CMD_APPEND_TO_FILE):
    case (CMD_FINISH_APPEND_TO_FILE):
    case (REPLY_READ_FINISHED_STOP):
    case (REPLY_FINISH_APPEND): {
      store_address_t storeId = GenericFileSystemMessage::getStoreId(message);
      auto ipcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
      if (ipcStore == nullptr) {
        return returnvalue::FAILED;
      }
      return ipcStore->deleteData(storeId);
    }
  }
  return returnvalue::OK;
}

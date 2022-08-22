#include "fsfw/action.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/storagemanager/StorageManagerIF.h"

ActionMessage::ActionMessage() = default;

ActionMessage::~ActionMessage() = default;

void ActionMessage::setCommand(CommandMessage* message, ActionId_t fid,
                               store_address_t parameters) {
  message->setCommand(EXECUTE_ACTION);
  message->setParameter(fid);
  message->setParameter2(parameters.raw);
}

ActionId_t ActionMessage::getActionId(const CommandMessage* message) {
  return ActionId_t(message->getParameter());
}

store_address_t ActionMessage::getStoreId(const CommandMessage* message) {
  store_address_t temp;
  temp.raw = message->getParameter2();
  return temp;
}

void ActionMessage::setStepReply(CommandMessage* message, ActionId_t fid, uint8_t step,
                                 ReturnValue_t result) {
  if (result == returnvalue::OK) {
    message->setCommand(STEP_SUCCESS);
  } else {
    message->setCommand(STEP_FAILED);
  }
  message->setParameter(fid);
  message->setParameter2((step << 16) + result);
}

uint8_t ActionMessage::getStep(const CommandMessage* message) {
  return uint8_t((message->getParameter2() >> 16) & 0xFF);
}

ReturnValue_t ActionMessage::getReturnCode(const CommandMessage* message) {
  return message->getParameter2() & 0xFFFF;
}

void ActionMessage::setDataReply(CommandMessage* message, ActionId_t actionId,
                                 store_address_t data) {
  message->setCommand(DATA_REPLY);
  message->setParameter(actionId);
  message->setParameter2(data.raw);
}

void ActionMessage::setCompletionReply(CommandMessage* message, ActionId_t fid, bool success,
                                       ReturnValue_t result) {
  if (success) {
    message->setCommand(COMPLETION_SUCCESS);
  } else {
    message->setCommand(COMPLETION_FAILED);
  }
  message->setParameter(fid);
  message->setParameter2(result);
}

void ActionMessage::clear(CommandMessage* message) {
  switch (message->getCommand()) {
    case EXECUTE_ACTION:
    case DATA_REPLY: {
      auto* ipcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
      if (ipcStore != nullptr) {
        ipcStore->deleteData(getStoreId(message));
      }
      break;
    }
    default:
      break;
  }
}

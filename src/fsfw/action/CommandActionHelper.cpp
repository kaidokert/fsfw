#include "fsfw/action.h"
#include "fsfw/objectmanager/ObjectManager.h"

CommandActionHelper::CommandActionHelper(CommandsActionsIF *setOwner)
    : owner(setOwner), queueToUse(nullptr), ipcStore(nullptr), commandCount(0), lastTarget(0) {}

CommandActionHelper::~CommandActionHelper() = default;

ReturnValue_t CommandActionHelper::commandAction(object_id_t commandTo, ActionId_t actionId,
                                                 SerializeIF *data) {
  auto *receiver = ObjectManager::instance()->get<HasActionsIF>(commandTo);
  if (receiver == nullptr) {
    return CommandsActionsIF::OBJECT_HAS_NO_FUNCTIONS;
  }
  store_address_t storeId;
  uint8_t *storePointer;
  size_t maxSize = data->getSerializedSize();
  ReturnValue_t result = ipcStore->getFreeElement(&storeId, maxSize, &storePointer);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  size_t size = 0;
  result = data->serialize(&storePointer, &size, maxSize, SerializeIF::Endianness::BIG);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  return sendCommand(receiver->getCommandQueue(), actionId, storeId);
}

ReturnValue_t CommandActionHelper::commandAction(object_id_t commandTo, ActionId_t actionId,
                                                 const uint8_t *data, uint32_t size) {
  auto *receiver = ObjectManager::instance()->get<HasActionsIF>(commandTo);
  if (receiver == nullptr) {
    return CommandsActionsIF::OBJECT_HAS_NO_FUNCTIONS;
  }
  store_address_t storeId;
  ReturnValue_t result = ipcStore->addData(&storeId, data, size);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  return sendCommand(receiver->getCommandQueue(), actionId, storeId);
}

ReturnValue_t CommandActionHelper::sendCommand(MessageQueueId_t queueId, ActionId_t actionId,
                                               store_address_t storeId) {
  CommandMessage command;
  ActionMessage::setCommand(&command, actionId, storeId);
  ReturnValue_t result = queueToUse->sendMessage(queueId, &command);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    ipcStore->deleteData(storeId);
  }
  lastTarget = queueId;
  commandCount++;
  return result;
}

ReturnValue_t CommandActionHelper::initialize() {
  ipcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
  if (ipcStore == nullptr) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }

  queueToUse = owner->getCommandQueuePtr();
  if (queueToUse == nullptr) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t CommandActionHelper::handleReply(CommandMessage *reply) {
  if (reply->getSender() != lastTarget) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  switch (reply->getCommand()) {
    case ActionMessage::COMPLETION_SUCCESS:
      commandCount--;
      owner->completionSuccessfulReceived(ActionMessage::getActionId(reply));
      return HasReturnvaluesIF::RETURN_OK;
    case ActionMessage::COMPLETION_FAILED:
      commandCount--;
      owner->completionFailedReceived(ActionMessage::getActionId(reply),
                                      ActionMessage::getReturnCode(reply));
      return HasReturnvaluesIF::RETURN_OK;
    case ActionMessage::STEP_SUCCESS:
      owner->stepSuccessfulReceived(ActionMessage::getActionId(reply),
                                    ActionMessage::getStep(reply));
      return HasReturnvaluesIF::RETURN_OK;
    case ActionMessage::STEP_FAILED:
      commandCount--;
      owner->stepFailedReceived(ActionMessage::getActionId(reply), ActionMessage::getStep(reply),
                                ActionMessage::getReturnCode(reply));
      return HasReturnvaluesIF::RETURN_OK;
    case ActionMessage::DATA_REPLY:
      extractDataForOwner(ActionMessage::getActionId(reply), ActionMessage::getStoreId(reply));
      return HasReturnvaluesIF::RETURN_OK;
    default:
      return HasReturnvaluesIF::RETURN_FAILED;
  }
}

uint8_t CommandActionHelper::getCommandCount() const { return commandCount; }

void CommandActionHelper::extractDataForOwner(ActionId_t actionId, store_address_t storeId) {
  const uint8_t *data = nullptr;
  size_t size = 0;
  ReturnValue_t result = ipcStore->getData(storeId, &data, &size);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return;
  }
  owner->dataReceived(actionId, data, size);
  ipcStore->deleteData(storeId);
}

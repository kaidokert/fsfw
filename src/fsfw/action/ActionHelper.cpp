#include "fsfw/action.h"
#include "fsfw/ipc/MessageQueueSenderIF.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

ActionHelper::ActionHelper(HasActionsIF* setOwner, MessageQueueIF* useThisQueue)
    : owner(setOwner), queueToUse(useThisQueue) {}

ActionHelper::~ActionHelper() {}

ReturnValue_t ActionHelper::handleActionMessage(CommandMessage* command) {
  if (command->getCommand() == ActionMessage::EXECUTE_ACTION) {
    ActionId_t currentAction = ActionMessage::getActionId(command);
    prepareExecution(command->getSender(), currentAction, ActionMessage::getStoreId(command));
    return HasReturnvaluesIF::RETURN_OK;
  } else {
    return CommandMessage::UNKNOWN_COMMAND;
  }
}

ReturnValue_t ActionHelper::initialize(MessageQueueIF* queueToUse_) {
  ipcStore = ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
  if (ipcStore == nullptr) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  if (queueToUse_ != nullptr) {
    setQueueToUse(queueToUse_);
  }

  if (queueToUse == nullptr) {
#if FSFW_VERBOSE_LEVEL >= 1
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "ActionHelper::initialize: No queue set" << std::endl;
#else
    sif::printWarning("ActionHelper::initialize: No queue set\n");
#endif
#endif /* FSFW_VERBOSE_LEVEL >= 1 */
    return HasReturnvaluesIF::RETURN_FAILED;
  }

  return HasReturnvaluesIF::RETURN_OK;
}

void ActionHelper::step(uint8_t step, MessageQueueId_t reportTo, ActionId_t commandId,
                        ReturnValue_t result) {
  CommandMessage reply;
  ActionMessage::setStepReply(&reply, commandId, step + STEP_OFFSET, result);
  queueToUse->sendMessage(reportTo, &reply);
}

void ActionHelper::finish(bool success, MessageQueueId_t reportTo, ActionId_t commandId,
                          ReturnValue_t result) {
  CommandMessage reply;
  ActionMessage::setCompletionReply(&reply, commandId, success, result);
  queueToUse->sendMessage(reportTo, &reply);
}

void ActionHelper::setQueueToUse(MessageQueueIF* queue) { queueToUse = queue; }

void ActionHelper::prepareExecution(MessageQueueId_t commandedBy, ActionId_t actionId,
                                    store_address_t dataAddress) {
  const uint8_t* dataPtr = NULL;
  size_t size = 0;
  ReturnValue_t result = ipcStore->getData(dataAddress, &dataPtr, &size);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    CommandMessage reply;
    ActionMessage::setStepReply(&reply, actionId, 0, result);
    queueToUse->sendMessage(commandedBy, &reply);
    return;
  }
  result = owner->executeAction(actionId, commandedBy, dataPtr, size);
  ipcStore->deleteData(dataAddress);
  if (result == HasActionsIF::EXECUTION_FINISHED) {
    CommandMessage reply;
    ActionMessage::setCompletionReply(&reply, actionId, true, result);
    queueToUse->sendMessage(commandedBy, &reply);
  }
  if (result != HasReturnvaluesIF::RETURN_OK) {
    CommandMessage reply;
    ActionMessage::setStepReply(&reply, actionId, 0, result);
    queueToUse->sendMessage(commandedBy, &reply);
    return;
  }
}

ReturnValue_t ActionHelper::reportData(MessageQueueId_t reportTo, ActionId_t replyId,
                                       SerializeIF* data, bool hideSender) {
  CommandMessage reply;
  store_address_t storeAddress;
  uint8_t* dataPtr;
  size_t maxSize = data->getSerializedSize();
  if (maxSize == 0) {
    /* No error, there's simply nothing to report. */
    return HasReturnvaluesIF::RETURN_OK;
  }
  size_t size = 0;
  ReturnValue_t result = ipcStore->getFreeElement(&storeAddress, maxSize, &dataPtr);
  if (result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "ActionHelper::reportData: Getting free element from IPC store failed!"
                 << std::endl;
#else
    sif::printWarning(
        "ActionHelper::reportData: Getting free element from IPC "
        "store failed!\n");
#endif
    return result;
  }
  result = data->serialize(&dataPtr, &size, maxSize, SerializeIF::Endianness::BIG);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    ipcStore->deleteData(storeAddress);
    return result;
  }

  /* We don't need to report the objectId, as we receive REQUESTED data before the completion
  success message. True aperiodic replies need to be reported with another dedicated message. */
  ActionMessage::setDataReply(&reply, replyId, storeAddress);

  /* If the sender needs to be hidden, for example to handle packet
  as unrequested reply, this will be done here. */
  if (hideSender) {
    result = MessageQueueSenderIF::sendMessage(reportTo, &reply);
  } else {
    result = queueToUse->sendMessage(reportTo, &reply);
  }

  if (result != HasReturnvaluesIF::RETURN_OK) {
    ipcStore->deleteData(storeAddress);
  }
  return result;
}

void ActionHelper::resetHelper() {}

ReturnValue_t ActionHelper::reportData(MessageQueueId_t reportTo, ActionId_t replyId,
                                       const uint8_t* data, size_t dataSize, bool hideSender) {
  CommandMessage reply;
  store_address_t storeAddress;
  ReturnValue_t result = ipcStore->addData(&storeAddress, data, dataSize);
  if (result != HasReturnvaluesIF::RETURN_OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "ActionHelper::reportData: Adding data to IPC store failed!" << std::endl;
#else
    sif::printWarning("ActionHelper::reportData: Adding data to IPC store failed!\n");
#endif
    return result;
  }

  /* We don't need to report the objectId, as we receive REQUESTED data before the completion
  success message. True aperiodic replies need to be reported with another dedicated message. */
  ActionMessage::setDataReply(&reply, replyId, storeAddress);

  /* If the sender needs to be hidden, for example to handle packet
  as unrequested reply, this will be done here. */
  if (hideSender) {
    result = MessageQueueSenderIF::sendMessage(reportTo, &reply);
  } else {
    result = queueToUse->sendMessage(reportTo, &reply);
  }

  if (result != HasReturnvaluesIF::RETURN_OK) {
    ipcStore->deleteData(storeAddress);
  }
  return result;
}

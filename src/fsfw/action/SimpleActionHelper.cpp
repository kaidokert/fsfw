#include "fsfw/action.h"

SimpleActionHelper::SimpleActionHelper(HasActionsIF* setOwner, MessageQueueIF* useThisQueue)
    : ActionHelper(setOwner, useThisQueue), isExecuting(false) {}

SimpleActionHelper::~SimpleActionHelper() = default;

void SimpleActionHelper::step(ReturnValue_t result) {
  // STEP_OFFESET is subtracted to compensate for adding offset in base
  // method, which is not necessary here.
  ActionHelper::step(stepCount - STEP_OFFSET, lastCommander, lastAction, result);
  if (result != returnvalue::OK) {
    resetHelper();
  }
}

void SimpleActionHelper::finish(ReturnValue_t result) {
  ActionHelper::finish(lastCommander, lastAction, result);
  resetHelper();
}

ReturnValue_t SimpleActionHelper::reportData(SerializeIF* data) {
  return ActionHelper::reportData(lastCommander, lastAction, data);
}

void SimpleActionHelper::resetHelper() {
  stepCount = 0;
  isExecuting = false;
  lastAction = 0;
  lastCommander = 0;
}

void SimpleActionHelper::prepareExecution(MessageQueueId_t commandedBy, ActionId_t actionId,
                                          store_address_t dataAddress) {
  CommandMessage reply;
  if (isExecuting) {
    ipcStore->deleteData(dataAddress);
    ActionMessage::setStepReply(&reply, actionId, 0, HasActionsIF::IS_BUSY);
    queueToUse->sendMessage(commandedBy, &reply);
  }
  const uint8_t* dataPtr = nullptr;
  size_t size = 0;
  ReturnValue_t result = ipcStore->getData(dataAddress, &dataPtr, &size);
  if (result != returnvalue::OK) {
    ActionMessage::setStepReply(&reply, actionId, 0, result);
    queueToUse->sendMessage(commandedBy, &reply);
    return;
  }
  lastCommander = commandedBy;
  lastAction = actionId;
  result = owner->executeAction(actionId, commandedBy, dataPtr, size);
  ipcStore->deleteData(dataAddress);
  switch (result) {
    case returnvalue::OK:
      isExecuting = true;
      stepCount++;
      break;
    case HasActionsIF::EXECUTION_FINISHED:
      ActionMessage::setCompletionReply(&reply, actionId, true, returnvalue::OK);
      queueToUse->sendMessage(commandedBy, &reply);
      break;
    default:
      ActionMessage::setStepReply(&reply, actionId, 0, result);
      queueToUse->sendMessage(commandedBy, &reply);
      break;
  }
}

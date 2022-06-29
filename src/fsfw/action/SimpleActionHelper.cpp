#include "fsfw/action.h"

SimpleActionHelper::SimpleActionHelper(HasActionsIF* setOwner, MessageQueueIF* useThisQueue)
    : ActionHelper(setOwner, useThisQueue), isExecuting(false) {}

SimpleActionHelper::~SimpleActionHelper() {}

void SimpleActionHelper::step(ReturnValue_t result) {
  // STEP_OFFESET is subtracted to compensate for adding offset in base
  // method, which is not necessary here.
  ActionHelper::step(stepCount - STEP_OFFSET, lastCommander, lastAction, result);
  if (result != HasReturnvaluesIF::RETURN_OK) {
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
  const uint8_t* dataPtr = NULL;
  size_t size = 0;
  ReturnValue_t result = ipcStore->getData(dataAddress, &dataPtr, &size);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    ActionMessage::setStepReply(&reply, actionId, 0, result);
    queueToUse->sendMessage(commandedBy, &reply);
    ipcStore->deleteData(dataAddress);
    return;
  }
  auto actionIter = actionMap.find(actionId);
  if (actionIter == actionMap.end()){
    CommandMessage reply;
    ActionMessage::setStepReply(&reply, actionId, 0, HasActionsIF::INVALID_ACTION_ID);
    queueToUse->sendMessage(commandedBy, &reply);
    ipcStore->deleteData(dataAddress);
    return;
  }
  Action* action = actionIter->second;
  result = action->deSerialize(&dataPtr, &size, SerializeIF::Endianness::NETWORK);
  if (result != HasReturnvaluesIF::RETURN_OK){
    CommandMessage reply;
    ActionMessage::setStepReply(&reply, actionId, 0, HasActionsIF::INVALID_PARAMETERS);
    queueToUse->sendMessage(commandedBy, &reply);
    ipcStore->deleteData(dataAddress);
    return;
  }
  result = action->handle();
  ipcStore->deleteData(dataAddress);
  switch (result) {
    case HasReturnvaluesIF::RETURN_OK:
      isExecuting = true;
      stepCount++;
      break;
    case HasActionsIF::EXECUTION_FINISHED:
      ActionMessage::setCompletionReply(&reply, actionId, true, HasReturnvaluesIF::RETURN_OK);
      queueToUse->sendMessage(commandedBy, &reply);
      break;
    default:
      ActionMessage::setStepReply(&reply, actionId, 0, result);
      queueToUse->sendMessage(commandedBy, &reply);
      break;
  }
}

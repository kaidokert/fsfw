#include "fsfw/controller/ExtendedControllerBase.h"

ExtendedControllerBase::ExtendedControllerBase(object_id_t objectId, object_id_t parentId,
                                               size_t commandQueueDepth)
    : ControllerBase(objectId, parentId, commandQueueDepth),
      poolManager(this, commandQueue),
      actionHelper(this, commandQueue) {}

ExtendedControllerBase::~ExtendedControllerBase() {}

ActionHelper *ExtendedControllerBase::getActionHelper() {
  return &actionHelper;
}

ReturnValue_t ExtendedControllerBase::executeAction(Action *action) {
  return action->handle();
}

object_id_t ExtendedControllerBase::getObjectId() const { return SystemObject::getObjectId(); }

uint32_t ExtendedControllerBase::getPeriodicOperationFrequency() const {
  return this->executingTask->getPeriodMs();
}

ReturnValue_t ExtendedControllerBase::handleCommandMessage(CommandMessage *message) {
  ReturnValue_t result = actionHelper.handleActionMessage(message);
  if (result == HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  return poolManager.handleHousekeepingMessage(message);
}

void ExtendedControllerBase::handleQueue() {
  CommandMessage command;
  ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
  for (result = commandQueue->receiveMessage(&command); result == RETURN_OK;
       result = commandQueue->receiveMessage(&command)) {
    result = actionHelper.handleActionMessage(&command);
    if (result == RETURN_OK) {
      continue;
    }

    result = modeHelper.handleModeCommand(&command);
    if (result == RETURN_OK) {
      continue;
    }

    result = healthHelper.handleHealthCommand(&command);
    if (result == RETURN_OK) {
      continue;
    }

    result = poolManager.handleHousekeepingMessage(&command);
    if (result == RETURN_OK) {
      continue;
    }

    result = handleCommandMessage(&command);
    if (result == RETURN_OK) {
      continue;
    }
    command.setToUnknownCommand();
    commandQueue->reply(&command);
  }
}

ReturnValue_t ExtendedControllerBase::initialize() {
  ReturnValue_t result = ControllerBase::initialize();
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  result = actionHelper.initialize(commandQueue);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }

  return poolManager.initialize(commandQueue);
}

ReturnValue_t ExtendedControllerBase::initializeAfterTaskCreation() {
  return poolManager.initializeAfterTaskCreation();
}

ReturnValue_t ExtendedControllerBase::performOperation(uint8_t opCode) {
  handleQueue();
  performControlOperation();
  /* We do this after performing control operation because variables will be set changed
  in this function. */
  poolManager.performHkOperation();
  return RETURN_OK;
}

MessageQueueId_t ExtendedControllerBase::getCommandQueue() const { return commandQueue->getId(); }

LocalDataPoolManager *ExtendedControllerBase::getHkManagerHandle() { return &poolManager; }

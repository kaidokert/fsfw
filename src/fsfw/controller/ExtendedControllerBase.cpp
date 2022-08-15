#include "fsfw/controller/ExtendedControllerBase.h"

ExtendedControllerBase::ExtendedControllerBase(object_id_t objectId, object_id_t parentId,
                                               size_t commandQueueDepth)
    : ControllerBase(objectId, parentId, commandQueueDepth),
      poolManager(this, commandQueue),
      actionHelper(this, commandQueue) {}

ExtendedControllerBase::~ExtendedControllerBase() = default;

ReturnValue_t ExtendedControllerBase::executeAction(ActionId_t actionId,
                                                    MessageQueueId_t commandedBy,
                                                    const uint8_t *data, size_t size) {
  /* Needs to be overriden and implemented by child class. */
  return returnvalue::OK;
}

object_id_t ExtendedControllerBase::getObjectId() const { return SystemObject::getObjectId(); }

uint32_t ExtendedControllerBase::getPeriodicOperationFrequency() const {
  return this->executingTask->getPeriodMs();
}

ReturnValue_t ExtendedControllerBase::handleCommandMessage(CommandMessage *message) {
  ReturnValue_t result = actionHelper.handleActionMessage(message);
  if (result == returnvalue::OK) {
    return result;
  }
  return poolManager.handleHousekeepingMessage(message);
}

void ExtendedControllerBase::handleQueue() {
  CommandMessage command;
  ReturnValue_t result;
  for (result = commandQueue->receiveMessage(&command); result == returnvalue::OK;
       result = commandQueue->receiveMessage(&command)) {
    result = actionHelper.handleActionMessage(&command);
    if (result == returnvalue::OK) {
      continue;
    }

    result = modeHelper.handleModeCommand(&command);
    if (result == returnvalue::OK) {
      continue;
    }

    result = healthHelper.handleHealthCommand(&command);
    if (result == returnvalue::OK) {
      continue;
    }

    result = poolManager.handleHousekeepingMessage(&command);
    if (result == returnvalue::OK) {
      continue;
    }

    result = handleCommandMessage(&command);
    if (result == returnvalue::OK) {
      continue;
    }
    command.setToUnknownCommand();
    commandQueue->reply(&command);
  }
}

ReturnValue_t ExtendedControllerBase::initialize() {
  ReturnValue_t result = ControllerBase::initialize();
  if (result != returnvalue::OK) {
    return result;
  }
  result = actionHelper.initialize(commandQueue);
  if (result != returnvalue::OK) {
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
  return returnvalue::OK;
}

MessageQueueId_t ExtendedControllerBase::getCommandQueue() const { return commandQueue->getId(); }

LocalDataPoolManager *ExtendedControllerBase::getHkManagerHandle() { return &poolManager; }

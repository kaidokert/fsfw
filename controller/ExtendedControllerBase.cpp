#include "ExtendedControllerBase.h"


ExtendedControllerBase::ExtendedControllerBase(object_id_t objectId,
        object_id_t parentId, size_t commandQueueDepth):
        ControllerBase(objectId, parentId, commandQueueDepth),
        localPoolManager(this, commandQueue),
        actionHelper(this, commandQueue) {
}

ReturnValue_t ExtendedControllerBase::executeAction(ActionId_t actionId,
        MessageQueueId_t commandedBy, const uint8_t *data, size_t size) {
    // needs to be overriden and implemented by child class.
    return HasReturnvaluesIF::RETURN_OK;
}



ReturnValue_t ExtendedControllerBase::initializeLocalDataPool(
        LocalDataPool &localDataPoolMap, LocalDataPoolManager &poolManager) {
    // needs to be overriden and implemented by child class.
    return HasReturnvaluesIF::RETURN_OK;
}

object_id_t ExtendedControllerBase::getObjectId() const {
    return SystemObject::getObjectId();
}

LocalDataPoolManager* ExtendedControllerBase::getHkManagerHandle() {
    return &localPoolManager;
}

uint32_t ExtendedControllerBase::getPeriodicOperationFrequency() const {
    return this->executingTask->getPeriodMs();
}

ReturnValue_t ExtendedControllerBase::handleCommandMessage(
        CommandMessage *message) {
    ReturnValue_t result = actionHelper.handleActionMessage(message);
    if(result == HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    return localPoolManager.handleHousekeepingMessage(message);
}

void ExtendedControllerBase::handleQueue() {
    CommandMessage command;
    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
    for (result = commandQueue->receiveMessage(&command);
            result == RETURN_OK;
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

        result = localPoolManager.handleHousekeepingMessage(&command);
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
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }
    result = actionHelper.initialize(commandQueue);
    if(result != HasReturnvaluesIF::RETURN_OK) {
        return result;
    }

    return localPoolManager.initialize(commandQueue);
}

ReturnValue_t ExtendedControllerBase::initializeAfterTaskCreation() {
    return localPoolManager.initializeAfterTaskCreation();
}

ReturnValue_t ExtendedControllerBase::performOperation(uint8_t opCode) {
    handleQueue();
    localPoolManager.performHkOperation();
    performControlOperation();
    return RETURN_OK;
}

MessageQueueId_t ExtendedControllerBase::getCommandQueue() const {
    return commandQueue->getId();
}

LocalPoolDataSetBase* ExtendedControllerBase::getDataSetHandle(sid_t sid) {
#if CPP_OSTREAM_ENABLED == 1
    sif::warning << "ExtendedControllerBase::getDataSetHandle: No child "
            << " implementation provided, returning nullptr!" << std::endl;
#endif
    return nullptr;
}

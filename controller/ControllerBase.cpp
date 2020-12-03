#include "ControllerBase.h"

#include "../subsystem/SubsystemBase.h"
#include "../ipc/QueueFactory.h"
#include "../action/HasActionsIF.h"

ControllerBase::ControllerBase(object_id_t setObjectId, object_id_t parentId,
		size_t commandQueueDepth) :
		SystemObject(setObjectId), parentId(parentId), mode(MODE_OFF),
		submode(SUBMODE_NONE), modeHelper(this),
		healthHelper(this, setObjectId), hkSwitcher(this) {
	commandQueue = QueueFactory::instance()->createMessageQueue(
	        commandQueueDepth);
}

ControllerBase::~ControllerBase() {
	QueueFactory::instance()->deleteMessageQueue(commandQueue);
}

ReturnValue_t ControllerBase::initialize() {
	ReturnValue_t result = SystemObject::initialize();
	if (result != RETURN_OK) {
		return result;
	}

	MessageQueueId_t parentQueue = 0;
	if (parentId != objects::NO_OBJECT) {
		SubsystemBase *parent = objectManager->get<SubsystemBase>(parentId);
		if (parent == nullptr) {
			return RETURN_FAILED;
		}
		parentQueue = parent->getCommandQueue();

		parent->registerChild(getObjectId());
	}

	result = healthHelper.initialize(parentQueue);
	if (result != RETURN_OK) {
		return result;
	}

	result = modeHelper.initialize(parentQueue);
	if (result != RETURN_OK) {
		return result;
	}

	result = hkSwitcher.initialize();
	if (result != RETURN_OK) {
		return result;
	}
	return RETURN_OK;
}

MessageQueueId_t ControllerBase::getCommandQueue() const {
	return commandQueue->getId();
}

void ControllerBase::handleQueue() {
	CommandMessage command;
	ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
	for (result = commandQueue->receiveMessage(&command);
	        result == RETURN_OK;
			result = commandQueue->receiveMessage(&command)) {

		result = modeHelper.handleModeCommand(&command);
		if (result == RETURN_OK) {
			continue;
		}

		result = healthHelper.handleHealthCommand(&command);
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

void ControllerBase::startTransition(Mode_t mode, Submode_t submode) {
	changeHK(this->mode, this->submode, false);
	triggerEvent(CHANGING_MODE, mode, submode);
	this->mode = mode;
	this->submode = submode;
	modeHelper.modeChanged(mode, submode);
	modeChanged(mode, submode);
	announceMode(false);
	changeHK(this->mode, this->submode, true);
}

void ControllerBase::getMode(Mode_t* mode, Submode_t* submode) {
	*mode = this->mode;
	*submode = this->submode;
}

void ControllerBase::setToExternalControl() {
	healthHelper.setHealth(EXTERNAL_CONTROL);
}

void ControllerBase::announceMode(bool recursive) {
	triggerEvent(MODE_INFO, mode, submode);
}

ReturnValue_t ControllerBase::performOperation(uint8_t opCode) {
	handleQueue();
	hkSwitcher.performOperation();
	performControlOperation();
	return RETURN_OK;
}

void ControllerBase::modeChanged(Mode_t mode, Submode_t submode) {
	return;
}

ReturnValue_t ControllerBase::setHealth(HealthState health) {
	switch (health) {
	case HEALTHY:
	case EXTERNAL_CONTROL:
		healthHelper.setHealth(health);
		return RETURN_OK;
	default:
		return INVALID_HEALTH_STATE;
	}
}

HasHealthIF::HealthState ControllerBase::getHealth() {
	return healthHelper.getHealth();
}
void ControllerBase::setTaskIF(PeriodicTaskIF* task_){
			executingTask = task_;
}

void ControllerBase::changeHK(Mode_t mode, Submode_t submode, bool enable) {
}

ReturnValue_t ControllerBase::initializeAfterTaskCreation() {
    return HasReturnvaluesIF::RETURN_OK;
}

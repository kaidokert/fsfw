#include "fsfw/controller/ControllerBase.h"

#include "fsfw/action/HasActionsIF.h"
#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/subsystem/SubsystemBase.h"

ControllerBase::ControllerBase(object_id_t setObjectId, object_id_t parentId,
                               size_t commandQueueDepth)
    : SystemObject(setObjectId),
      parentId(parentId),
      mode(MODE_OFF),
      submode(SUBMODE_NONE),
      modeHelper(this),
      healthHelper(this, setObjectId) {
  commandQueue = QueueFactory::instance()->createMessageQueue(commandQueueDepth);
}

ControllerBase::~ControllerBase() { QueueFactory::instance()->deleteMessageQueue(commandQueue); }

ReturnValue_t ControllerBase::initialize() {
  ReturnValue_t result = SystemObject::initialize();
  if (result != returnvalue::OK) {
    return result;
  }

  MessageQueueId_t parentQueue = 0;
  if (parentId != objects::NO_OBJECT) {
    auto* parent = ObjectManager::instance()->get<SubsystemBase>(parentId);
    if (parent == nullptr) {
      return returnvalue::FAILED;
    }
    parentQueue = parent->getCommandQueue();

    parent->registerChild(getObjectId());
  }

  result = healthHelper.initialize(parentQueue);
  if (result != returnvalue::OK) {
    return result;
  }

  result = modeHelper.initialize(parentQueue);
  if (result != returnvalue::OK) {
    return result;
  }

  return returnvalue::OK;
}

MessageQueueId_t ControllerBase::getCommandQueue() const { return commandQueue->getId(); }

void ControllerBase::handleQueue() {
  CommandMessage command;
  ReturnValue_t result;
  for (result = commandQueue->receiveMessage(&command); result == returnvalue::OK;
       result = commandQueue->receiveMessage(&command)) {
    result = modeHelper.handleModeCommand(&command);
    if (result == returnvalue::OK) {
      continue;
    }

    result = healthHelper.handleHealthCommand(&command);
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

void ControllerBase::startTransition(Mode_t mode_, Submode_t submode_) {
  changeHK(this->mode, this->submode, false);
  triggerEvent(CHANGING_MODE, mode, submode);
  mode = mode_;
  submode = submode_;
  modeHelper.modeChanged(mode, submode);
  modeChanged(mode, submode);
  announceMode(false);
  changeHK(this->mode, this->submode, true);
}

void ControllerBase::getMode(Mode_t* mode_, Submode_t* submode_) {
  *mode_ = this->mode;
  *submode_ = this->submode;
}

void ControllerBase::setToExternalControl() { healthHelper.setHealth(EXTERNAL_CONTROL); }

void ControllerBase::announceMode(bool recursive) { triggerEvent(MODE_INFO, mode, submode); }

ReturnValue_t ControllerBase::performOperation(uint8_t opCode) {
  handleQueue();
  performControlOperation();
  return returnvalue::OK;
}

void ControllerBase::modeChanged(Mode_t mode_, Submode_t submode_) {}

ReturnValue_t ControllerBase::setHealth(HealthState health) {
  switch (health) {
    case HEALTHY:
    case EXTERNAL_CONTROL:
      healthHelper.setHealth(health);
      return returnvalue::OK;
    default:
      return INVALID_HEALTH_STATE;
  }
}

HasHealthIF::HealthState ControllerBase::getHealth() { return healthHelper.getHealth(); }
void ControllerBase::setTaskIF(PeriodicTaskIF* task_) { executingTask = task_; }

void ControllerBase::changeHK(Mode_t mode_, Submode_t submode_, bool enable) {}

ReturnValue_t ControllerBase::initializeAfterTaskCreation() { return returnvalue::OK; }

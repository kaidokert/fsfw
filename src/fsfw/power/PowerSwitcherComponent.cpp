#include "PowerSwitcherComponent.h"

#include <fsfw/ipc/QueueFactory.h>
#include <fsfw/power/PowerSwitchIF.h>

PowerSwitcherComponent::PowerSwitcherComponent(object_id_t objectId, PowerSwitchIF *pwrSwitcher,
                                               power::Switch_t pwrSwitch)
    : SystemObject(objectId),
      switcher(pwrSwitcher, pwrSwitch),
      modeHelper(this),
      healthHelper(this, objectId) {
  queue = QueueFactory::instance()->createMessageQueue();
}

ReturnValue_t PowerSwitcherComponent::performOperation(uint8_t opCode) {
  ReturnValue_t result;
  CommandMessage command;

  for (result = queue->receiveMessage(&command); result == returnvalue::OK;
       result = queue->receiveMessage(&command)) {
    result = healthHelper.handleHealthCommand(&command);
    if (result == returnvalue::OK) {
      continue;
    }

    result = modeHelper.handleModeCommand(&command);
    if (result == returnvalue::OK) {
      continue;
    }
  }
  if (switcher.active()) {
    switcher.doStateMachine();
    auto currState = switcher.getState();
    if (currState == PowerSwitcher::SWITCH_IS_OFF) {
      setMode(MODE_OFF, 0);
    } else if (currState == PowerSwitcher::SWITCH_IS_ON) {
      setMode(MODE_ON, 0);
    }
  }
  return returnvalue::OK;
}

ReturnValue_t PowerSwitcherComponent::initialize() {
  ReturnValue_t result = modeHelper.initialize();
  if (result != returnvalue::OK) {
    return result;
  }
  result = healthHelper.initialize();
  if (result != returnvalue::OK) {
    return result;
  }
  return SystemObject::initialize();
}

MessageQueueId_t PowerSwitcherComponent::getCommandQueue() const { return queue->getId(); }

void PowerSwitcherComponent::getMode(Mode_t *mode, Submode_t *submode) {
  *mode = this->mode;
  *submode = this->submode;
}

ReturnValue_t PowerSwitcherComponent::setHealth(HealthState health) {
  healthHelper.setHealth(health);
  return returnvalue::OK;
}

ReturnValue_t PowerSwitcherComponent::checkModeCommand(Mode_t mode, Submode_t submode,
                                                       uint32_t *msToReachTheMode) {
  *msToReachTheMode = 5000;
  if (mode != MODE_ON and mode != MODE_OFF) {
    return TRANS_NOT_ALLOWED;
  }
  return returnvalue::OK;
}

void PowerSwitcherComponent::startTransition(Mode_t mode, Submode_t submode) {
  if (mode == MODE_OFF) {
    switcher.turnOff(true);
    switcher.doStateMachine();
    if (switcher.getState() == PowerSwitcher::SWITCH_IS_OFF) {
      setMode(MODE_OFF, 0);
    }
  } else if (mode == MODE_ON) {
    switcher.turnOn(true);
    switcher.doStateMachine();
    if (switcher.getState() == PowerSwitcher::SWITCH_IS_ON) {
      setMode(MODE_ON, 0);
    }
  }
}

void PowerSwitcherComponent::setToExternalControl() {
  healthHelper.setHealth(HasHealthIF::EXTERNAL_CONTROL);
}

void PowerSwitcherComponent::announceMode(bool recursive) {
  triggerEvent(MODE_INFO, mode, submode);
}

void PowerSwitcherComponent::setMode(Mode_t newMode, Submode_t newSubmode) {
  this->mode = newMode;
  this->submode = newSubmode;
  modeHelper.modeChanged(mode, submode);
  announceMode(false);
}

HasHealthIF::HealthState PowerSwitcherComponent::getHealth() { return healthHelper.getHealth(); }

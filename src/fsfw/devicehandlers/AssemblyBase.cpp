#include "fsfw/devicehandlers/AssemblyBase.h"

AssemblyBase::AssemblyBase(object_id_t objectId, object_id_t parentId, uint16_t commandQueueDepth)
    : SubsystemBase(objectId, parentId, MODE_OFF, commandQueueDepth),
      internalState(STATE_NONE),
      recoveryState(RECOVERY_IDLE),
      recoveringDevice(childrenMap.end()),
      targetMode(MODE_OFF),
      targetSubmode(SUBMODE_NONE) {
  recoveryOffTimer.setTimeout(POWER_OFF_TIME_MS);
}

AssemblyBase::~AssemblyBase() {}

ReturnValue_t AssemblyBase::handleCommandMessage(CommandMessage* message) {
  return handleHealthReply(message);
}

void AssemblyBase::performChildOperation() {
  if (isInTransition()) {
    handleChildrenTransition();
  } else {
    handleChildrenChanged();
  }
}

void AssemblyBase::startTransition(Mode_t mode, Submode_t submode) {
  doStartTransition(mode, submode);
  if (modeHelper.isForced()) {
    triggerEvent(FORCING_MODE, mode, submode);
  } else {
    triggerEvent(CHANGING_MODE, mode, submode);
  }
}

void AssemblyBase::doStartTransition(Mode_t mode, Submode_t submode) {
  targetMode = mode;
  targetSubmode = submode;
  internalState = STATE_SINGLE_STEP;
  ReturnValue_t result = commandChildren(mode, submode);
  if (result == NEED_SECOND_STEP) {
    internalState = STATE_NEED_SECOND_STEP;
  }
}

bool AssemblyBase::isInTransition() {
  return (internalState != STATE_NONE) || (recoveryState != RECOVERY_IDLE);
}

bool AssemblyBase::handleChildrenChanged() {
  if (childrenChangedMode) {
    ReturnValue_t result = checkChildrenState();
    if (result != returnvalue::OK) {
      handleChildrenLostMode(result);
    }
    return true;
  } else {
    return handleChildrenChangedHealth();
  }
}

void AssemblyBase::handleChildrenLostMode(ReturnValue_t result) {
  triggerEvent(CANT_KEEP_MODE, mode, submode);
  startTransition(MODE_OFF, SUBMODE_NONE);
}

bool AssemblyBase::handleChildrenChangedHealth() {
  auto iter = childrenMap.begin();
  for (; iter != childrenMap.end(); iter++) {
    if (iter->second.healthChanged) {
      iter->second.healthChanged = false;
      break;
    }
  }
  if (iter == childrenMap.end()) {
    return false;
  }
  HealthState healthState = healthHelper.healthTable->getHealth(iter->first);
  if (healthState == HasHealthIF::NEEDS_RECOVERY) {
    triggerEvent(TRYING_RECOVERY);
    recoveryState = RECOVERY_STARTED;
    recoveringDevice = iter;
    doStartTransition(targetMode, targetSubmode);
  } else {
    triggerEvent(CHILD_CHANGED_HEALTH);
    doStartTransition(mode, submode);
  }
  if (modeHelper.isForced()) {
    triggerEvent(FORCING_MODE, targetMode, targetSubmode);
  }
  return true;
}

void AssemblyBase::handleChildrenTransition() {
  if (commandsOutstanding <= 0) {
    switch (internalState) {
      case STATE_NEED_SECOND_STEP:
        internalState = STATE_SECOND_STEP;
        commandChildren(targetMode, targetSubmode);
        return;
      case STATE_OVERWRITE_HEALTH: {
        internalState = STATE_SINGLE_STEP;
        ReturnValue_t result = commandChildren(mode, submode);
        if (result == NEED_SECOND_STEP) {
          internalState = STATE_NEED_SECOND_STEP;
        }
        return;
      }
      case STATE_NONE:
        // Valid state, used in recovery.
      case STATE_SINGLE_STEP:
      case STATE_SECOND_STEP:
        if (checkAndHandleRecovery()) {
          return;
        }
        break;
    }
    ReturnValue_t result = checkChildrenState();
    if (result == returnvalue::OK) {
      handleModeReached();
    } else {
      handleModeTransitionFailed(result);
    }
  }
}

void AssemblyBase::handleModeReached() {
  internalState = STATE_NONE;
  setMode(targetMode, targetSubmode);
}

void AssemblyBase::handleModeTransitionFailed(ReturnValue_t result) {
  // always accept transition to OFF, there is nothing we can do except sending an info event
  // In theory this should never happen, but we would risk an infinite loop otherwise
  if (targetMode == MODE_OFF) {
    triggerEvent(CHILD_PROBLEMS, result);
    internalState = STATE_NONE;
    setMode(targetMode, targetSubmode);
  } else {
    if (handleChildrenChangedHealth()) {
      // If any health change is pending, handle that first.
      return;
    }
    triggerEvent(MODE_TRANSITION_FAILED, result);
    startTransition(MODE_OFF, SUBMODE_NONE);
  }
}

void AssemblyBase::sendHealthCommand(MessageQueueId_t sendTo, HealthState health) {
  CommandMessage command;
  HealthMessage::setHealthMessage(&command, HealthMessage::HEALTH_SET, health);
  if (commandQueue->sendMessage(sendTo, &command) == returnvalue::OK) {
    commandsOutstanding++;
  }
}

ReturnValue_t AssemblyBase::checkChildrenState() {
  if (targetMode == MODE_OFF) {
    return checkChildrenStateOff();
  } else {
    return checkChildrenStateOn(targetMode, targetSubmode);
  }
}

ReturnValue_t AssemblyBase::checkChildrenStateOff() {
  for (const auto& childIter : childrenMap) {
    if (checkChildOff(childIter.first) != returnvalue::OK) {
      return NOT_ENOUGH_CHILDREN_IN_CORRECT_STATE;
    }
  }
  return returnvalue::OK;
}

ReturnValue_t AssemblyBase::checkChildOff(uint32_t objectId) {
  ChildInfo childInfo = childrenMap.find(objectId)->second;
  if (healthHelper.healthTable->isCommandable(objectId)) {
    if (childInfo.submode != SUBMODE_NONE) {
      return returnvalue::FAILED;
    } else {
      if ((childInfo.mode != MODE_OFF) && (childInfo.mode != DeviceHandlerIF::MODE_ERROR_ON)) {
        return returnvalue::FAILED;
      }
    }
  }
  return returnvalue::OK;
}

ReturnValue_t AssemblyBase::checkModeCommand(Mode_t mode, Submode_t submode,
                                             uint32_t* msToReachTheMode) {
  // always accept transition to OFF
  if (mode == MODE_OFF) {
    if (submode != SUBMODE_NONE) {
      return INVALID_SUBMODE;
    }
    return returnvalue::OK;
  }

  if ((mode != MODE_ON) && (mode != DeviceHandlerIF::MODE_NORMAL)) {
    return INVALID_MODE;
  }

  if (internalState != STATE_NONE) {
    return IN_TRANSITION;
  }

  return isModeCombinationValid(mode, submode);
}

ReturnValue_t AssemblyBase::handleHealthReply(CommandMessage* message) {
  if (message->getCommand() == HealthMessage::HEALTH_INFO) {
    HealthState health = HealthMessage::getHealth(message);
    if (health != EXTERNAL_CONTROL) {
      updateChildChangedHealth(message->getSender(), true);
    }
    return returnvalue::OK;
  }
  if (message->getCommand() == HealthMessage::REPLY_HEALTH_SET ||
      (message->getCommand() == CommandMessage::REPLY_REJECTED &&
       message->getParameter2() == HealthMessage::HEALTH_SET)) {
    if (isInTransition()) {
      commandsOutstanding--;
    }
    return returnvalue::OK;
  }
  return returnvalue::FAILED;
}

bool AssemblyBase::checkAndHandleRecovery() {
  switch (recoveryState) {
    case RECOVERY_STARTED:
      recoveryState = RECOVERY_WAIT;
      recoveryOffTimer.resetTimer();
      return true;
    case RECOVERY_WAIT:
      if (recoveryOffTimer.isBusy()) {
        return true;
      }
      triggerEvent(RECOVERY_STEP, 0);
      sendHealthCommand(recoveringDevice->second.commandQueue, HEALTHY);
      internalState = STATE_NONE;
      recoveryState = RECOVERY_ONGOING;
      // Don't check state!
      return true;
    case RECOVERY_ONGOING:
      triggerEvent(RECOVERY_STEP, 1);
      recoveryState = RECOVERY_ONGOING_2;
      recoveringDevice->second.healthChanged = false;
      // Device should be healthy again, so restart a transition.
      // Might be including second step, but that's already handled.
      doStartTransition(targetMode, targetSubmode);
      return true;
    case RECOVERY_ONGOING_2:
      triggerEvent(RECOVERY_DONE);
      // Now we're through, but not sure if it was successful.
      recoveryState = RECOVERY_IDLE;
      return false;
    case RECOVERY_IDLE:
    default:
      return false;
  }
}

void AssemblyBase::overwriteDeviceHealth(object_id_t objectId, HasHealthIF::HealthState oldHealth) {
  triggerEvent(OVERWRITING_HEALTH, objectId, oldHealth);
  internalState = STATE_OVERWRITE_HEALTH;
  modeHelper.setForced(true);
  sendHealthCommand(childrenMap[objectId].commandQueue, EXTERNAL_CONTROL);
}

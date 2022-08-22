#include "fsfw/thermal/Heater.h"

#include "fsfw/devicehandlers/DeviceHandlerFailureIsolation.h"
#include "fsfw/ipc/QueueFactory.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/power/Fuse.h"

Heater::Heater(uint32_t objectId, uint8_t switch0, uint8_t switch1)
    : HealthDevice(objectId, 0),
      internalState(STATE_OFF),
      switch0(switch0),
      switch1(switch1),
      heaterOnCountdown(10800000) /*about two orbits*/,
      parameterHelper(this) {
  eventQueue = QueueFactory::instance()->createMessageQueue();
}

Heater::~Heater() { QueueFactory::instance()->deleteMessageQueue(eventQueue); }

ReturnValue_t Heater::set() {
  passive = false;
  // wait for clear before doing anything
  if (internalState == STATE_WAIT) {
    return returnvalue::OK;
  }
  if (healthHelper.healthTable->isHealthy(getObjectId())) {
    doAction(SET);
    if ((internalState == STATE_OFF) || (internalState == STATE_PASSIVE)) {
      return returnvalue::FAILED;
    } else {
      return returnvalue::OK;
    }
  } else {
    if (healthHelper.healthTable->isFaulty(getObjectId())) {
      if (!reactedToBeingFaulty) {
        reactedToBeingFaulty = true;
        doAction(CLEAR);
      }
    }
    return returnvalue::FAILED;
  }
}

void Heater::clear(bool passive) {
  this->passive = passive;
  // Force switching off
  if (internalState == STATE_WAIT) {
    internalState = STATE_ON;
  }
  if (healthHelper.healthTable->isHealthy(getObjectId())) {
    doAction(CLEAR);
  } else if (healthHelper.healthTable->isFaulty(getObjectId())) {
    if (!reactedToBeingFaulty) {
      reactedToBeingFaulty = true;
      doAction(CLEAR);
    }
  }
}

void Heater::doAction(Action action) {
  // only act if we are not in the right state or in a transition
  if (action == SET) {
    if ((internalState == STATE_OFF) || (internalState == STATE_PASSIVE) ||
        (internalState == STATE_EXTERNAL_CONTROL)) {
      switchCountdown.setTimeout(powerSwitcher->getSwitchDelayMs());
      internalState = STATE_WAIT_FOR_SWITCHES_ON;
      powerSwitcher->sendSwitchCommand(switch0, PowerSwitchIF::SWITCH_ON);
      powerSwitcher->sendSwitchCommand(switch1, PowerSwitchIF::SWITCH_ON);
    }
  } else {  // clear
    if ((internalState == STATE_ON) || (internalState == STATE_FAULTY) ||
        (internalState == STATE_EXTERNAL_CONTROL)) {
      internalState = STATE_WAIT_FOR_SWITCHES_OFF;
      switchCountdown.setTimeout(powerSwitcher->getSwitchDelayMs());
      powerSwitcher->sendSwitchCommand(switch0, PowerSwitchIF::SWITCH_OFF);
      powerSwitcher->sendSwitchCommand(switch1, PowerSwitchIF::SWITCH_OFF);
    }
  }
}

void Heater::setPowerSwitcher(PowerSwitchIF* powerSwitch) { this->powerSwitcher = powerSwitch; }

ReturnValue_t Heater::performOperation(uint8_t opCode) {
  handleQueue();
  handleEventQueue();

  if (!healthHelper.healthTable->isFaulty(getObjectId())) {
    reactedToBeingFaulty = false;
  }

  switch (internalState) {
    case STATE_ON:
      if ((powerSwitcher->getSwitchState(switch0) == PowerSwitchIF::SWITCH_OFF) ||
          (powerSwitcher->getSwitchState(switch1) == PowerSwitchIF::SWITCH_OFF)) {
        // switch went off on its own
        // trigger event. FDIR can confirm if it is caused by MniOps and decide on the action
        // do not trigger FD events when under external control
        if (healthHelper.getHealth() != EXTERNAL_CONTROL) {
          triggerEvent(PowerSwitchIF::SWITCH_WENT_OFF);
        } else {
          internalState = STATE_EXTERNAL_CONTROL;
        }
      }
      break;
    case STATE_OFF:
      // check if heater is on, ie both switches are on
      // if so, just command it to off, to resolve the situation or force a switch stayed on event
      // But, only do anything if not already faulty (state off is the stable point for being
      // faulty)
      if ((!healthHelper.healthTable->isFaulty(getObjectId())) &&
          (powerSwitcher->getSwitchState(switch0) == PowerSwitchIF::SWITCH_ON) &&
          (powerSwitcher->getSwitchState(switch1) == PowerSwitchIF::SWITCH_ON)) {
        // do not trigger FD events when under external control
        if (healthHelper.getHealth() != EXTERNAL_CONTROL) {
          internalState = STATE_WAIT_FOR_SWITCHES_OFF;
          switchCountdown.setTimeout(powerSwitcher->getSwitchDelayMs());
          powerSwitcher->sendSwitchCommand(switch0, PowerSwitchIF::SWITCH_OFF);
          powerSwitcher->sendSwitchCommand(switch1, PowerSwitchIF::SWITCH_OFF);
        } else {
          internalState = STATE_EXTERNAL_CONTROL;
        }
      }
      break;
    case STATE_PASSIVE:
      break;
    case STATE_WAIT_FOR_SWITCHES_ON:
      if (switchCountdown.hasTimedOut()) {
        if ((powerSwitcher->getSwitchState(switch0) == PowerSwitchIF::SWITCH_OFF) ||
            (powerSwitcher->getSwitchState(switch1) == PowerSwitchIF::SWITCH_OFF)) {
          triggerEvent(HEATER_STAYED_OFF);
          internalState = STATE_WAIT_FOR_FDIR;  // wait before retrying or anything
        } else {
          triggerEvent(HEATER_ON);
          internalState = STATE_ON;
        }
      }
      break;
    case STATE_WAIT_FOR_SWITCHES_OFF:
      if (switchCountdown.hasTimedOut()) {
        // only check for both being on (ie heater still on)
        if ((powerSwitcher->getSwitchState(switch0) == PowerSwitchIF::SWITCH_ON) &&
            (powerSwitcher->getSwitchState(switch1) == PowerSwitchIF::SWITCH_ON)) {
          if (healthHelper.healthTable->isFaulty(getObjectId())) {
            if (passive) {
              internalState = STATE_PASSIVE;
            } else {
              internalState = STATE_OFF;  // just accept it
            }
            triggerEvent(HEATER_ON);  // but throw an event to make it more visible
            break;
          }
          triggerEvent(HEATER_STAYED_ON);
          internalState = STATE_WAIT_FOR_FDIR;  // wait before retrying or anything
        } else {
          triggerEvent(HEATER_OFF);
          if (passive) {
            internalState = STATE_PASSIVE;
          } else {
            internalState = STATE_OFF;
          }
        }
      }
      break;
    default:
      break;
  }

  if ((powerSwitcher->getSwitchState(switch0) == PowerSwitchIF::SWITCH_ON) &&
      (powerSwitcher->getSwitchState(switch1) == PowerSwitchIF::SWITCH_ON)) {
    if (wasOn) {
      if (heaterOnCountdown.hasTimedOut()) {
        // SHOULDDO this means if a heater fails in single mode, the timeout will start again
        // I am not sure if this is a bug, but atm I have no idea how to fix this and think
        // it will be ok. whatcouldpossiblygowrong™
        if (!timedOut) {
          triggerEvent(HEATER_TIMEOUT);
          timedOut = true;
        }
      }
    } else {
      wasOn = true;
      heaterOnCountdown.resetTimer();
      timedOut = false;
    }
  } else {
    wasOn = false;
  }

  return returnvalue::OK;
}

void Heater::setSwitch(uint8_t number, ReturnValue_t state, uint32_t* uptimeOfSwitching) {
  if (powerSwitcher == NULL) {
    return;
  }
  if (powerSwitcher->getSwitchState(number) == state) {
    *uptimeOfSwitching = INVALID_UPTIME;
  } else {
    if ((*uptimeOfSwitching == INVALID_UPTIME)) {
      powerSwitcher->sendSwitchCommand(number, state);
      Clock::getUptime(uptimeOfSwitching);
    } else {
      uint32_t currentUptime;
      Clock::getUptime(&currentUptime);
      if (currentUptime - *uptimeOfSwitching > powerSwitcher->getSwitchDelayMs()) {
        *uptimeOfSwitching = INVALID_UPTIME;
        if (healthHelper.healthTable->isHealthy(getObjectId())) {
          if (state == PowerSwitchIF::SWITCH_ON) {
            triggerEvent(HEATER_STAYED_OFF);
          } else {
            triggerEvent(HEATER_STAYED_ON);
          }
        }
      }
    }
  }
}

MessageQueueId_t Heater::getCommandQueue() const { return commandQueue->getId(); }

ReturnValue_t Heater::initialize() {
  ReturnValue_t result = SystemObject::initialize();
  if (result != returnvalue::OK) {
    return result;
  }

  EventManagerIF* manager = ObjectManager::instance()->get<EventManagerIF>(objects::EVENT_MANAGER);
  if (manager == NULL) {
    return returnvalue::FAILED;
  }
  result = manager->registerListener(eventQueue->getId());
  if (result != returnvalue::OK) {
    return result;
  }

  ConfirmsFailuresIF* pcdu = ObjectManager::instance()->get<ConfirmsFailuresIF>(
      DeviceHandlerFailureIsolation::powerConfirmationId);
  if (pcdu == NULL) {
    return returnvalue::FAILED;
  }
  pcduQueueId = pcdu->getEventReceptionQueue();

  result = manager->subscribeToAllEventsFrom(eventQueue->getId(), getObjectId());
  if (result != returnvalue::OK) {
    return result;
  }

  result = parameterHelper.initialize();
  if (result != returnvalue::OK) {
    return result;
  }

  result = healthHelper.initialize();
  if (result != returnvalue::OK) {
    return result;
  }

  return returnvalue::OK;
}

void Heater::handleQueue() {
  CommandMessage command;
  ReturnValue_t result = commandQueue->receiveMessage(&command);
  if (result == returnvalue::OK) {
    result = healthHelper.handleHealthCommand(&command);
    if (result == returnvalue::OK) {
      return;
    }
    result = parameterHelper.handleParameterMessage(&command);
    if (result == returnvalue::OK) {
      return;
    }
  }
}

ReturnValue_t Heater::getParameter(uint8_t domainId, uint8_t uniqueId,
                                   ParameterWrapper* parameterWrapper,
                                   const ParameterWrapper* newValues, uint16_t startAtIndex) {
  if (domainId != DOMAIN_ID_BASE) {
    return INVALID_DOMAIN_ID;
  }
  switch (uniqueId) {
    case 0:
      parameterWrapper->set(heaterOnCountdown.timeout);
      break;
    default:
      return INVALID_IDENTIFIER_ID;
  }
  return returnvalue::OK;
}

void Heater::handleEventQueue() {
  EventMessage event;
  for (ReturnValue_t result = eventQueue->receiveMessage(&event); result == returnvalue::OK;
       result = eventQueue->receiveMessage(&event)) {
    switch (event.getMessageId()) {
      case EventMessage::EVENT_MESSAGE:
        switch (event.getEvent()) {
          case Fuse::FUSE_WENT_OFF:
          case HEATER_STAYED_OFF:
          // HEATER_STAYED_ON is a setting if faulty does not help, but we need to reach a stable
          // state and can check for being faulty before throwing this event again.
          case HEATER_STAYED_ON:
            if (healthHelper.healthTable->isCommandable(getObjectId())) {
              healthHelper.setHealth(HasHealthIF::FAULTY);
              internalState = STATE_FAULTY;
            }
            break;
          case PowerSwitchIF::SWITCH_WENT_OFF:
            internalState = STATE_WAIT;
            event.setMessageId(EventMessage::CONFIRMATION_REQUEST);
            if (pcduQueueId != 0) {
              eventQueue->sendMessage(pcduQueueId, &event);
            } else {
              healthHelper.setHealth(HasHealthIF::FAULTY);
              internalState = STATE_FAULTY;
            }
            break;
          default:
            return;
        }
        break;
      case EventMessage::YOUR_FAULT:
        healthHelper.setHealth(HasHealthIF::FAULTY);
        internalState = STATE_FAULTY;
        break;
      case EventMessage::MY_FAULT:
        // do nothing, we are already in STATE_WAIT and wait for a clear()
        break;
      default:
        return;
    }
  }
}

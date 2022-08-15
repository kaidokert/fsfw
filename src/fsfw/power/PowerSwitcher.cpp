#include "fsfw/power/PowerSwitcher.h"

#include "definitions.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

PowerSwitcher::PowerSwitcher(PowerSwitchIF* switcher, power::Switch_t setSwitch1,
                             power::Switch_t setSwitch2, PowerSwitcher::State_t setStartState)
    : power(switcher), state(setStartState), firstSwitch(setSwitch1), secondSwitch(setSwitch2) {}

ReturnValue_t PowerSwitcher::getStateOfSwitches() {
  SwitchReturn_t result = howManySwitches();

  switch (result) {
    case ONE_SWITCH:
      return power->getSwitchState(firstSwitch);
    case TWO_SWITCHES: {
      ReturnValue_t firstSwitchState = power->getSwitchState(firstSwitch);
      ReturnValue_t secondSwitchState = power->getSwitchState(firstSwitch);
      if ((firstSwitchState == PowerSwitchIF::SWITCH_ON) &&
          (secondSwitchState == PowerSwitchIF::SWITCH_ON)) {
        return PowerSwitchIF::SWITCH_ON;
      } else if ((firstSwitchState == PowerSwitchIF::SWITCH_OFF) &&
                 (secondSwitchState == PowerSwitchIF::SWITCH_OFF)) {
        return PowerSwitchIF::SWITCH_OFF;
      } else {
        return returnvalue::FAILED;
      }
    }
    default:
      return returnvalue::FAILED;
  }
}

void PowerSwitcher::commandSwitches(ReturnValue_t onOff) {
  SwitchReturn_t result = howManySwitches();
  switch (result) {
    case TWO_SWITCHES:
      power->sendSwitchCommand(secondSwitch, onOff);
      /* NO BREAK falls through*/
    case ONE_SWITCH:
      power->sendSwitchCommand(firstSwitch, onOff);
      break;
  }
  return;
}

void PowerSwitcher::turnOn(bool checkCurrentState) {
  if (checkCurrentState) {
    if (getStateOfSwitches() == PowerSwitchIF::SWITCH_ON) {
      state = SWITCH_IS_ON;
      return;
    }
  }
  commandSwitches(PowerSwitchIF::SWITCH_ON);
  state = WAIT_ON;
}

void PowerSwitcher::turnOff(bool checkCurrentState) {
  if (checkCurrentState) {
    if (getStateOfSwitches() == PowerSwitchIF::SWITCH_OFF) {
      state = SWITCH_IS_OFF;
      return;
    }
  }
  commandSwitches(PowerSwitchIF::SWITCH_OFF);
  state = WAIT_OFF;
}

bool PowerSwitcher::active() {
  if (state == WAIT_OFF or state == WAIT_ON) {
    return true;
  }
  return false;
}

PowerSwitcher::SwitchReturn_t PowerSwitcher::howManySwitches() {
  if (secondSwitch == power::NO_SWITCH) {
    return ONE_SWITCH;
  } else {
    return TWO_SWITCHES;
  }
}

void PowerSwitcher::doStateMachine() {
  switch (state) {
    case SWITCH_IS_OFF:
    case SWITCH_IS_ON:
      // Do nothing.
      break;
    case WAIT_OFF:
      if (getStateOfSwitches() == PowerSwitchIF::SWITCH_OFF) {
        state = SWITCH_IS_OFF;
      }
      break;
    case WAIT_ON:
      if (getStateOfSwitches() == PowerSwitchIF::SWITCH_ON) {
        state = SWITCH_IS_ON;
      }
      break;
    default:
      // Should never happen.
      break;
  }
}

ReturnValue_t PowerSwitcher::checkSwitchState() {
  switch (state) {
    case WAIT_OFF:
    case WAIT_ON:
      return IN_POWER_TRANSITION;
    case SWITCH_IS_OFF:
      if (getStateOfSwitches() == PowerSwitchIF::SWITCH_OFF) {
        return returnvalue::OK;
      } else {
        return SWITCH_STATE_MISMATCH;
      }
    case SWITCH_IS_ON:
      if (getStateOfSwitches() == PowerSwitchIF::SWITCH_ON) {
        return returnvalue::OK;
      } else {
        return SWITCH_STATE_MISMATCH;
      }
  }
  return returnvalue::FAILED;
}

PowerSwitcher::State_t PowerSwitcher::getState() { return state; }

uint32_t PowerSwitcher::getSwitchDelay() { return power->getSwitchDelayMs(); }

uint8_t PowerSwitcher::getFirstSwitch() const { return firstSwitch; }

uint8_t PowerSwitcher::getSecondSwitch() const { return secondSwitch; }

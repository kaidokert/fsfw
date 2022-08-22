#ifndef FSFW_POWER_POWERSWITCHER_H_
#define FSFW_POWER_POWERSWITCHER_H_

#include "../objectmanager/SystemObjectIF.h"
#include "../returnvalues/returnvalue.h"
#include "../timemanager/Countdown.h"
#include "PowerSwitchIF.h"

class PowerSwitcher {
 public:
  enum State_t {
    WAIT_OFF,
    WAIT_ON,
    SWITCH_IS_OFF,
    SWITCH_IS_ON,
  };

  static const uint8_t INTERFACE_ID = CLASS_ID::POWER_SWITCHER;
  static const ReturnValue_t IN_POWER_TRANSITION = MAKE_RETURN_CODE(1);
  static const ReturnValue_t SWITCH_STATE_MISMATCH = MAKE_RETURN_CODE(2);
  PowerSwitcher(PowerSwitchIF* switcher, power::Switch_t setSwitch1,
                power::Switch_t setSwitch2 = power::NO_SWITCH,
                State_t setStartState = SWITCH_IS_OFF);
  void turnOn(bool checkCurrentState = true);
  void turnOff(bool checkCurrentState = true);
  bool active();
  void doStateMachine();
  State_t getState();
  ReturnValue_t checkSwitchState();
  uint32_t getSwitchDelay();
  power::Switch_t getFirstSwitch() const;
  power::Switch_t getSecondSwitch() const;

 private:
  PowerSwitchIF* power = nullptr;
  State_t state;
  power::Switch_t firstSwitch = power::NO_SWITCH;
  power::Switch_t secondSwitch = power::NO_SWITCH;

  enum SwitchReturn_t { ONE_SWITCH = 1, TWO_SWITCHES = 2 };
  ReturnValue_t getStateOfSwitches();
  void commandSwitches(ReturnValue_t onOff);
  SwitchReturn_t howManySwitches();
};

#endif /* FSFW_POWER_POWERSWITCHER_H_ */

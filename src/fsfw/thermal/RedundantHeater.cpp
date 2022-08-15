#include "fsfw/thermal/RedundantHeater.h"

RedundantHeater::~RedundantHeater() {}

RedundantHeater::RedundantHeater(Parameters parameters)
    : heater0(parameters.objectIdHeater0, parameters.switch0Heater0, parameters.switch1Heater0),
      heater1(parameters.objectIdHeater1, parameters.switch0Heater1, parameters.switch1Heater1) {}

void RedundantHeater::performOperation(uint8_t opCode) {
  heater0.performOperation(0);
  heater1.performOperation(0);
}

void RedundantHeater::set(bool on, bool both, bool passive) {
  if (on) {
    ReturnValue_t result = heater0.set();
    if (result != returnvalue::OK || both) {
      heater1.set();
    } else {
      heater1.clear(passive);
    }
  } else {
    heater0.clear(passive);
    heater1.clear(passive);
  }
}

void RedundantHeater::triggerHeaterEvent(Event event) {
  heater0.triggerEvent(event);
  heater1.triggerEvent(event);
}

void RedundantHeater::setPowerSwitcher(PowerSwitchIF* powerSwitch) {
  heater0.setPowerSwitcher(powerSwitch);
  heater1.setPowerSwitcher(powerSwitch);
}

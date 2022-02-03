#ifndef REDUNDANTHEATER_H_
#define REDUNDANTHEATER_H_

#include "../thermal/Heater.h"

class RedundantHeater {
 public:
  struct Parameters {
    Parameters(uint32_t objectIdHeater0, uint32_t objectIdHeater1, uint8_t switch0Heater0,
               uint8_t switch1Heater0, uint8_t switch0Heater1, uint8_t switch1Heater1)
        : objectIdHeater0(objectIdHeater0),
          objectIdHeater1(objectIdHeater1),
          switch0Heater0(switch0Heater0),
          switch1Heater0(switch1Heater0),
          switch0Heater1(switch0Heater1),
          switch1Heater1(switch1Heater1) {}

    Parameters()
        : objectIdHeater0(0),
          objectIdHeater1(0),
          switch0Heater0(0),
          switch1Heater0(0),
          switch0Heater1(0),
          switch1Heater1(0) {}

    uint32_t objectIdHeater0;
    uint32_t objectIdHeater1;
    uint8_t switch0Heater0;
    uint8_t switch1Heater0;
    uint8_t switch0Heater1;
    uint8_t switch1Heater1;
  };

  RedundantHeater(Parameters parameters);
  virtual ~RedundantHeater();

  void performOperation(uint8_t opCode);

  void triggerHeaterEvent(Event event);

  void set(bool on, bool both, bool passive = false);

  void setPowerSwitcher(PowerSwitchIF *powerSwitch);

 protected:
  Heater heater0;
  Heater heater1;
};

#endif /* REDUNDANTHEATER_H_ */

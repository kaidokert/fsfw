#ifndef FSFW_POWER_POWERCOMPONENTIF_H_
#define FSFW_POWER_POWERCOMPONENTIF_H_

#include "../objectmanager/SystemObjectIF.h"
#include "../parameters/HasParametersIF.h"
#include "../serialize/SerializeIF.h"

class PowerComponentIF : public SerializeIF, public HasParametersIF {
 public:
  virtual ~PowerComponentIF() {}

  virtual object_id_t getDeviceObjectId() = 0;

  virtual uint8_t getSwitchId1() = 0;
  virtual uint8_t getSwitchId2() = 0;
  virtual bool hasTwoSwitches() = 0;

  virtual float getMin() = 0;
  virtual float getMax() = 0;
};

#endif /* FSFW_POWER_POWERCOMPONENTIF_H_ */

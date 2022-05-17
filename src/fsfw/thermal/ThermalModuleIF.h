#ifndef THERMALMODULEIF_H_
#define THERMALMODULEIF_H_

#include "ThermalComponentIF.h"

class AbstractTemperatureSensor;

class ThermalModuleIF {
 public:
  enum Strategy {
    PASSIVE = 0,
    ACTIVE_SURVIVAL = 1,
    ACTIVE_SINGLE = 2,
    ACTIVE_DUAL = 3,
  };

  enum StateRequest { STATE_REQUEST_HEATING = 1, STATE_REQUEST_PASSIVE = 0 };

  enum State { NON_OPERATIONAL = 0, OPERATIONAL = 1, UNKNOWN = 2 };

  virtual ~ThermalModuleIF() {}

  virtual void performOperation(uint8_t opCode) = 0;

  virtual void performMode(Strategy strategy) = 0;

  virtual float getTemperature() = 0;

  virtual void registerSensor(AbstractTemperatureSensor *sensor) = 0;

  virtual void registerComponent(ThermalComponentIF *component,
                                 ThermalComponentIF::Priority priority) = 0;

  virtual ThermalComponentIF *findComponent(object_id_t objectId) = 0;

  virtual void setHeating(bool on) = 0;

  virtual void setOutputInvalid() = 0;
};

#endif /* THERMALMODULEIF_H_ */

#ifndef FSFW_THERMAL_THERMALMODULE_H_
#define FSFW_THERMAL_THERMALMODULE_H_

#include <list>

#include "../datapoollocal/LocalPoolDataSetBase.h"
#include "../datapoollocal/LocalPoolVariable.h"
#include "../devicehandlers/HealthDevice.h"
#include "../events/EventReportingProxyIF.h"
#include "RedundantHeater.h"
#include "ThermalModuleIF.h"
#include "tcsDefinitions.h"

class PowerSwitchIF;

/**
 * @brief Allows creation of different thermal control domains within a system.
 */
class ThermalModule : public ThermalModuleIF {
  friend class ThermalController;

 public:
  struct Parameters {
    float heaterOn;
    float hysteresis;
  };

  ThermalModule(gp_id_t moduleTemperaturePoolId, gp_id_t currentStatePoolId,
                gp_id_t targetStatePoolId, LocalPoolDataSetBase *dataSet, Parameters parameters,
                RedundantHeater::Parameters heaterParameters);

  ThermalModule(gp_id_t moduleTemperaturePoolId, LocalPoolDataSetBase *dataSet);

  virtual ~ThermalModule();

  void performOperation(uint8_t opCode);

  void performMode(Strategy strategy);

  float getTemperature();

  void registerSensor(AbstractTemperatureSensor *sensor);

  void registerComponent(ThermalComponentIF *component, ThermalComponentIF::Priority priority);

  ThermalComponentIF *findComponent(object_id_t objectId);

  void initialize(PowerSwitchIF *powerSwitch);

  void setHeating(bool on);

  virtual void setOutputInvalid();

 protected:
  enum Informee { ALL, SAFE, NONE };

  struct ComponentData {
    ThermalComponentIF *component;
    ThermalComponentIF::Priority priority;
    ThermalComponentIF::HeaterRequest request;
  };

  Strategy oldStrategy;

  float survivalTargetTemp = 0.0;

  float targetTemp = 0.0;

  bool heating = false;

  Parameters parameters;

  lp_var_t<float> moduleTemperature;

  RedundantHeater *heater = nullptr;

  lp_var_t<int8_t> currentState;
  lp_var_t<int8_t> targetState;

  std::list<AbstractTemperatureSensor *> sensors;
  std::list<ComponentData> components;

  void calculateTemperature();

  ThermalComponentIF::HeaterRequest letComponentsPerformAndDeciceIfWeNeedToHeat(bool safeOnly);

  void informComponentsAboutHeaterState(bool heaterIsOn, Informee whomToInform);

  bool calculateModuleHeaterRequestAndSetModuleStatus(Strategy strategy);

  void updateTargetTemperatures(ThermalComponentIF *component, bool isSafe);
};

#endif /* FSFW_THERMAL_THERMALMODULE_H_ */

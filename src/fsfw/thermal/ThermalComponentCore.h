#ifndef FSFW_THERMAL_THERMALCOMPONENTCORE_H_
#define FSFW_THERMAL_THERMALCOMPONENTCORE_H_

#include "AbstractTemperatureSensor.h"
#include "ThermalComponentIF.h"
#include "ThermalModule.h"
#include "ThermalMonitorReporter.h"
#include "fsfw/datapoollocal/LocalPoolVariable.h"

/**
 * @brief
 * @details
 */
class ThermalComponentCore : public ThermalComponentIF {
 public:
  struct Parameters {
    float lowerOpLimit;
    float upperOpLimit;
    float heaterOn;
    float hysteresis;
    float heaterSwitchoff;
  };

  static const uint16_t COMPONENT_TEMP_CONFIRMATION = 5;

  /**
   * Some documentation
   * @param reportingObjectId
   * @param domainId
   * @param temperaturePoolId
   * @param targetStatePoolId
   * @param currentStatePoolId
   * @param requestPoolId
   * @param dataSet
   * @param parameters
   * @param initialTargetState
   */
  ThermalComponentCore(
      object_id_t reportingObjectId, uint8_t domainId, gp_id_t temperaturePoolId,
      gp_id_t targetStatePoolId, gp_id_t currentStatePoolId, gp_id_t requestPoolId,
      LocalPoolDataSetBase *dataSet, Parameters parameters,
      StateRequest initialTargetState = ThermalComponentIF::STATE_REQUEST_OPERATIONAL);

  void addSensor(AbstractTemperatureSensor *firstRedundantSensor);
  void addFirstRedundantSensor(AbstractTemperatureSensor *firstRedundantSensor);
  void addSecondRedundantSensor(AbstractTemperatureSensor *secondRedundantSensor);
  void addThermalModule(ThermalModule *thermalModule, Priority priority);

  void setPriority(Priority priority);

  virtual ~ThermalComponentCore();

  virtual HeaterRequest performOperation(uint8_t opCode);

  void markStateIgnored();

  object_id_t getObjectId();

  uint8_t getDomainId() const;

  virtual float getLowerOpLimit();

  ReturnValue_t setTargetState(int8_t newState);

  virtual void setOutputInvalid();

  virtual ReturnValue_t getParameter(uint8_t domainId, uint8_t uniqueId,
                                     ParameterWrapper *parameterWrapper,
                                     const ParameterWrapper *newValues, uint16_t startAtIndex);

 protected:
  AbstractTemperatureSensor *sensor = nullptr;
  AbstractTemperatureSensor *firstRedundantSensor = nullptr;
  AbstractTemperatureSensor *secondRedundantSensor = nullptr;
  ThermalModuleIF *thermalModule = nullptr;

  lp_var_t<float> temperature;
  lp_var_t<int8_t> targetState;
  lp_var_t<int8_t> currentState;
  lp_var_t<uint8_t> heaterRequest;

  bool isHeating = false;

  bool isSafeComponent = false;

  float minTemp = 999;

  float maxTemp = AbstractTemperatureSensor::ZERO_KELVIN_C;

  Parameters parameters;

  const uint8_t domainId;

  ThermalMonitorReporter temperatureMonitor;

  virtual float getTemperature();
  virtual State getState(float temperature, Parameters parameters, int8_t targetState);

  virtual void checkLimits(State state);

  virtual HeaterRequest getHeaterRequest(int8_t targetState, float temperature,
                                         Parameters parameters);

  virtual State getIgnoredState(int8_t state);

  void updateMinMaxTemp();

  virtual Parameters getParameters();
};

#endif /* FSFW_THERMAL_THERMALCOMPONENT_CORE_H_ */

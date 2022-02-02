#ifndef FSFW_THERMAL_THERMALCOMPONENT_H_
#define FSFW_THERMAL_THERMALCOMPONENT_H_

#include "ThermalComponentCore.h"

/**
 * @brief
 * @details
 * Some more documentation.
 */
class ThermalComponent : public ThermalComponentCore {
 public:
  struct Parameters {
    float lowerNopLimit;
    float lowerOpLimit;
    float upperOpLimit;
    float upperNopLimit;
    float heaterOn;
    float hysteresis;
    float heaterSwitchoff;
  };

  /**
   * Non-Operational Temperatures
   */
  struct NopParameters {
    float lowerNopLimit;
    float upperNopLimit;
  };

  /**
   * How to use.
   * @param reportingObjectId
   * @param domainId
   * @param temperaturePoolId
   * @param targetStatePoolId
   * @param currentStatePoolId
   * @param requestPoolId
   * @param dataSet
   * @param sensor
   * @param firstRedundantSensor
   * @param secondRedundantSensor
   * @param thermalModule
   * @param parameters
   * @param priority
   */
  ThermalComponent(object_id_t reportingObjectId, uint8_t domainId, gp_id_t temperaturePoolId,
                   gp_id_t targetStatePoolId, gp_id_t currentStatePoolId, gp_id_t requestPoolId,
                   LocalPoolDataSetBase *dataSet, AbstractTemperatureSensor *sensor,
                   AbstractTemperatureSensor *firstRedundantSensor,
                   AbstractTemperatureSensor *secondRedundantSensor, ThermalModuleIF *thermalModule,
                   Parameters parameters, Priority priority);
  virtual ~ThermalComponent();

  ReturnValue_t setTargetState(int8_t newState);

  virtual ReturnValue_t setLimits(const uint8_t *data, size_t size);

  virtual ReturnValue_t getParameter(uint8_t domainId, uint8_t uniqueId,
                                     ParameterWrapper *parameterWrapper,
                                     const ParameterWrapper *newValues, uint16_t startAtIndex);

 protected:
  NopParameters nopParameters;

  State getState(float temperature, ThermalComponentCore::Parameters parameters,
                 int8_t targetState);

  virtual void checkLimits(State state);

  virtual HeaterRequest getHeaterRequest(int8_t targetState, float temperature,
                                         ThermalComponentCore::Parameters parameters);

  State getIgnoredState(int8_t state);
};

#endif /* FSFW_THERMAL_THERMALCOMPONENT_H_ */

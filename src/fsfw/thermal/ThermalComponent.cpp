#include "fsfw/thermal/ThermalComponent.h"

ThermalComponent::ThermalComponent(object_id_t reportingObjectId, uint8_t domainId,
                                   gp_id_t temperaturePoolId, gp_id_t targetStatePoolId,
                                   gp_id_t currentStatePoolId, gp_id_t requestPoolId,
                                   LocalPoolDataSetBase* dataSet, AbstractTemperatureSensor* sensor,
                                   AbstractTemperatureSensor* firstRedundantSensor,
                                   AbstractTemperatureSensor* secondRedundantSensor,
                                   ThermalModuleIF* thermalModule, Parameters parameters,
                                   Priority priority)
    : ThermalComponentCore(reportingObjectId, domainId, temperaturePoolId, targetStatePoolId,
                           currentStatePoolId, requestPoolId, dataSet,
                           {parameters.lowerOpLimit, parameters.upperOpLimit, parameters.heaterOn,
                            parameters.hysteresis, parameters.heaterSwitchoff},
                           ThermalComponentIF::STATE_REQUEST_NON_OPERATIONAL),
      nopParameters({parameters.lowerNopLimit, parameters.upperNopLimit}) {}

ThermalComponent::~ThermalComponent() {}

ReturnValue_t ThermalComponent::setTargetState(int8_t newState) {
  targetState.setReadWriteMode(pool_rwm_t::VAR_READ_WRITE);
  targetState.read();
  if ((targetState == STATE_REQUEST_OPERATIONAL) and (newState != STATE_REQUEST_IGNORE)) {
    return returnvalue::FAILED;
  }
  switch (newState) {
    case STATE_REQUEST_NON_OPERATIONAL:
      targetState = newState;
      targetState.setValid(true);
      targetState.commit(PoolVariableIF::VALID);
      return returnvalue::OK;
    default:
      return ThermalComponentCore::setTargetState(newState);
  }
  return returnvalue::OK;
}

ReturnValue_t ThermalComponent::setLimits(const uint8_t* data, size_t size) {
  if (size != 4 * sizeof(parameters.lowerOpLimit)) {
    return MonitoringIF::INVALID_SIZE;
  }
  size_t readSize = size;
  SerializeAdapter::deSerialize(&nopParameters.lowerNopLimit, &data, &readSize,
                                SerializeIF::Endianness::BIG);
  SerializeAdapter::deSerialize(&parameters.lowerOpLimit, &data, &readSize,
                                SerializeIF::Endianness::BIG);
  SerializeAdapter::deSerialize(&parameters.upperOpLimit, &data, &readSize,
                                SerializeIF::Endianness::BIG);
  SerializeAdapter::deSerialize(&nopParameters.upperNopLimit, &data, &readSize,
                                SerializeIF::Endianness::BIG);
  return returnvalue::OK;
}

ThermalComponentIF::State ThermalComponent::getState(float temperature,
                                                     ThermalComponentCore::Parameters parameters,
                                                     int8_t targetState) {
  if (temperature < nopParameters.lowerNopLimit) {
    return OUT_OF_RANGE_LOW;
  } else {
    State state = ThermalComponentCore::getState(temperature, parameters, targetState);
    if (state != NON_OPERATIONAL_HIGH && state != NON_OPERATIONAL_HIGH_IGNORED) {
      return state;
    }
    if (temperature > nopParameters.upperNopLimit) {
      state = OUT_OF_RANGE_HIGH;
    }
    if (targetState == STATE_REQUEST_IGNORE) {
      state = getIgnoredState(state);
    }
    return state;
  }
}

void ThermalComponent::checkLimits(ThermalComponentIF::State state) {
  if ((targetState == STATE_REQUEST_OPERATIONAL) or (targetState == STATE_REQUEST_IGNORE)) {
    ThermalComponentCore::checkLimits(state);
    return;
  }
  // If component is not operational, it checks the NOP limits.
  temperatureMonitor.translateState(state, temperature.value, nopParameters.lowerNopLimit,
                                    nopParameters.upperNopLimit, false);
}

ThermalComponentIF::HeaterRequest ThermalComponent::getHeaterRequest(
    int8_t targetState, float temperature, ThermalComponentCore::Parameters parameters) {
  if (targetState == STATE_REQUEST_IGNORE) {
    isHeating = false;
    return HEATER_DONT_CARE;
  }

  if (temperature > nopParameters.upperNopLimit - parameters.heaterSwitchoff) {
    isHeating = false;
    return HEATER_REQUEST_EMERGENCY_OFF;
  }

  float nopHeaterLimit = nopParameters.lowerNopLimit + parameters.heaterOn;
  float opHeaterLimit = parameters.lowerOpLimit + parameters.heaterOn;

  if (isHeating) {
    nopHeaterLimit += parameters.hysteresis;
    opHeaterLimit += parameters.hysteresis;
  }

  if (temperature < nopHeaterLimit) {
    isHeating = true;
    return HEATER_REQUEST_EMERGENCY_ON;
  }

  if ((targetState == STATE_REQUEST_OPERATIONAL) || (targetState == STATE_REQUEST_HEATING)) {
    if (temperature < opHeaterLimit) {
      isHeating = true;
      return HEATER_REQUEST_ON;
    }
    if (temperature > parameters.upperOpLimit - parameters.heaterSwitchoff) {
      isHeating = false;
      return HEATER_REQUEST_OFF;
    }
  }

  isHeating = false;
  return HEATER_DONT_CARE;
}

ThermalComponentIF::State ThermalComponent::getIgnoredState(int8_t state) {
  switch (state) {
    case OUT_OF_RANGE_LOW:
      return OUT_OF_RANGE_LOW_IGNORED;
    case OUT_OF_RANGE_HIGH:
      return OUT_OF_RANGE_HIGH_IGNORED;
    case OUT_OF_RANGE_LOW_IGNORED:
      return OUT_OF_RANGE_LOW_IGNORED;
    case OUT_OF_RANGE_HIGH_IGNORED:
      return OUT_OF_RANGE_HIGH_IGNORED;
    default:
      return ThermalComponentCore::getIgnoredState(state);
  }
}

ReturnValue_t ThermalComponent::getParameter(uint8_t domainId, uint8_t uniqueId,
                                             ParameterWrapper* parameterWrapper,
                                             const ParameterWrapper* newValues,
                                             uint16_t startAtIndex) {
  ReturnValue_t result = ThermalComponentCore::getParameter(domainId, uniqueId, parameterWrapper,
                                                            newValues, startAtIndex);
  if (result != INVALID_IDENTIFIER_ID) {
    return result;
  }
  switch (uniqueId) {
    case 12:
      parameterWrapper->set(nopParameters.lowerNopLimit);
      break;
    case 13:
      parameterWrapper->set(nopParameters.upperNopLimit);
      break;
    default:
      return INVALID_IDENTIFIER_ID;
  }
  return returnvalue::OK;
}

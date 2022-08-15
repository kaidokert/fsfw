#include "fsfw/thermal/ThermalComponentCore.h"

#include "fsfw/thermal/tcsDefinitions.h"

ThermalComponentCore::ThermalComponentCore(object_id_t reportingObjectId, uint8_t domainId,
                                           gp_id_t temperaturePoolId, gp_id_t targetStatePoolId,
                                           gp_id_t currentStatePoolId, gp_id_t requestPoolId,
                                           LocalPoolDataSetBase* dataSet, Parameters parameters,
                                           StateRequest initialTargetState)
    : temperature(temperaturePoolId, dataSet, PoolVariableIF::VAR_WRITE),
      targetState(targetStatePoolId, dataSet, PoolVariableIF::VAR_READ),
      currentState(currentStatePoolId, dataSet, PoolVariableIF::VAR_WRITE),
      heaterRequest(requestPoolId, dataSet, PoolVariableIF::VAR_WRITE),
      parameters(parameters),
      domainId(domainId),
      temperatureMonitor(reportingObjectId, domainId + 1, temperaturePoolId,
                         COMPONENT_TEMP_CONFIRMATION) {
  // Set thermal state once, then leave to operator.
  targetState.setReadWriteMode(PoolVariableIF::VAR_WRITE);
  ReturnValue_t result = targetState.read();
  if (result == returnvalue::OK) {
    targetState = initialTargetState;
    targetState.setValid(true);
    targetState.commit();
  }
  targetState.setReadWriteMode(PoolVariableIF::VAR_READ);
}

void ThermalComponentCore::addSensor(AbstractTemperatureSensor* sensor) { this->sensor = sensor; }

void ThermalComponentCore::addFirstRedundantSensor(
    AbstractTemperatureSensor* firstRedundantSensor) {
  this->firstRedundantSensor = firstRedundantSensor;
}

void ThermalComponentCore::addSecondRedundantSensor(
    AbstractTemperatureSensor* secondRedundantSensor) {
  this->secondRedundantSensor = secondRedundantSensor;
}

void ThermalComponentCore::addThermalModule(ThermalModule* thermalModule, Priority priority) {
  this->thermalModule = thermalModule;
  if (thermalModule != nullptr) {
    thermalModule->registerComponent(this, priority);
  }
}

void ThermalComponentCore::setPriority(Priority priority) {
  if (priority == SAFE) {
    this->isSafeComponent = true;
  }
}

ThermalComponentCore::~ThermalComponentCore() {}

ThermalComponentIF::HeaterRequest ThermalComponentCore::performOperation(uint8_t opCode) {
  HeaterRequest request = HEATER_DONT_CARE;
  // SHOULDDO: Better pass db_float_t* to getTemperature and set it invalid if invalid.
  temperature = getTemperature();
  updateMinMaxTemp();
  if (temperature != thermal::INVALID_TEMPERATURE) {
    temperature.setValid(PoolVariableIF::VALID);
    State state = getState(temperature.value, getParameters(), targetState.value);
    currentState = state;
    checkLimits(state);
    request = getHeaterRequest(targetState.value, temperature.value, getParameters());
  } else {
    temperatureMonitor.setToInvalid();
    temperature.setValid(PoolVariableIF::INVALID);
    currentState = UNKNOWN;
    request = HEATER_DONT_CARE;
  }
  currentState.setValid(PoolVariableIF::VALID);
  heaterRequest = request;
  heaterRequest.setValid(PoolVariableIF::VALID);
  return request;
}

void ThermalComponentCore::markStateIgnored() {
  currentState = getIgnoredState(currentState.value);
}

object_id_t ThermalComponentCore::getObjectId() {
  return temperatureMonitor.getReporterId();
  return 0;
}

float ThermalComponentCore::getLowerOpLimit() { return parameters.lowerOpLimit; }

ReturnValue_t ThermalComponentCore::setTargetState(int8_t newState) {
  targetState.setReadWriteMode(pool_rwm_t::VAR_READ_WRITE);
  targetState.read();
  if ((targetState == STATE_REQUEST_OPERATIONAL) and (newState != STATE_REQUEST_IGNORE)) {
    return returnvalue::FAILED;
  }

  switch (newState) {
    case STATE_REQUEST_HEATING:
    case STATE_REQUEST_IGNORE:
    case STATE_REQUEST_OPERATIONAL:
      targetState = newState;
      break;
    case STATE_REQUEST_NON_OPERATIONAL:
    default:
      return INVALID_TARGET_STATE;
  }
  targetState.setValid(true);
  targetState.commit();
  return returnvalue::OK;
}

void ThermalComponentCore::setOutputInvalid() {
  temperature = thermal::INVALID_TEMPERATURE;
  temperature.setValid(PoolVariableIF::INVALID);
  currentState.setValid(PoolVariableIF::INVALID);
  heaterRequest = HEATER_DONT_CARE;
  heaterRequest.setValid(PoolVariableIF::INVALID);
  temperatureMonitor.setToUnchecked();
}

float ThermalComponentCore::getTemperature() {
  if ((sensor != nullptr) && (sensor->isValid())) {
    return sensor->getTemperature();
  }

  if ((firstRedundantSensor != nullptr) && (firstRedundantSensor->isValid())) {
    return firstRedundantSensor->getTemperature();
  }

  if ((secondRedundantSensor != nullptr) && (secondRedundantSensor->isValid())) {
    return secondRedundantSensor->getTemperature();
  }

  if (thermalModule != nullptr) {
    float temperature = thermalModule->getTemperature();
    if (temperature != thermal::INVALID_TEMPERATURE) {
      return temperature;
    } else {
      return thermal::INVALID_TEMPERATURE;
    }
  } else {
    return thermal::INVALID_TEMPERATURE;
  }
}

ThermalComponentIF::State ThermalComponentCore::getState(float temperature, Parameters parameters,
                                                         int8_t targetState) {
  ThermalComponentIF::State state;

  if (temperature < parameters.lowerOpLimit) {
    state = NON_OPERATIONAL_LOW;
  } else if (temperature < parameters.upperOpLimit) {
    state = OPERATIONAL;
  } else {
    state = NON_OPERATIONAL_HIGH;
  }
  if (targetState == STATE_REQUEST_IGNORE) {
    state = getIgnoredState(state);
  }

  return state;
}

void ThermalComponentCore::checkLimits(ThermalComponentIF::State state) {
  // Checks operational limits only.
  temperatureMonitor.translateState(state, temperature.value, getParameters().lowerOpLimit,
                                    getParameters().upperOpLimit);
}

ThermalComponentIF::HeaterRequest ThermalComponentCore::getHeaterRequest(int8_t targetState,
                                                                         float temperature,
                                                                         Parameters parameters) {
  if (targetState == STATE_REQUEST_IGNORE) {
    isHeating = false;
    return HEATER_DONT_CARE;
  }

  if (temperature > parameters.upperOpLimit - parameters.heaterSwitchoff) {
    isHeating = false;
    return HEATER_REQUEST_EMERGENCY_OFF;
  }

  float opHeaterLimit = parameters.lowerOpLimit + parameters.heaterOn;

  if (isHeating) {
    opHeaterLimit += parameters.hysteresis;
  }

  if (temperature < opHeaterLimit) {
    isHeating = true;
    return HEATER_REQUEST_EMERGENCY_ON;
  }
  isHeating = false;
  return HEATER_DONT_CARE;
}

ThermalComponentIF::State ThermalComponentCore::getIgnoredState(int8_t state) {
  switch (state) {
    case NON_OPERATIONAL_LOW:
      return NON_OPERATIONAL_LOW_IGNORED;
    case OPERATIONAL:
      return OPERATIONAL_IGNORED;
    case NON_OPERATIONAL_HIGH:
      return NON_OPERATIONAL_HIGH_IGNORED;
    case NON_OPERATIONAL_LOW_IGNORED:
      return NON_OPERATIONAL_LOW_IGNORED;
    case OPERATIONAL_IGNORED:
      return OPERATIONAL_IGNORED;
    case NON_OPERATIONAL_HIGH_IGNORED:
      return NON_OPERATIONAL_HIGH_IGNORED;
    default:
    case UNKNOWN:
      return UNKNOWN;
  }
}

void ThermalComponentCore::updateMinMaxTemp() {
  if (temperature == thermal::INVALID_TEMPERATURE) {
    return;
  }
  if (temperature < minTemp) {
    minTemp = static_cast<float>(temperature);
  }
  if (temperature > maxTemp) {
    maxTemp = static_cast<float>(temperature);
  }
}

uint8_t ThermalComponentCore::getDomainId() const { return domainId; }

ThermalComponentCore::Parameters ThermalComponentCore::getParameters() { return parameters; }

ReturnValue_t ThermalComponentCore::getParameter(uint8_t domainId, uint8_t uniqueId,
                                                 ParameterWrapper* parameterWrapper,
                                                 const ParameterWrapper* newValues,
                                                 uint16_t startAtIndex) {
  ReturnValue_t result = temperatureMonitor.getParameter(domainId, uniqueId, parameterWrapper,
                                                         newValues, startAtIndex);
  if (result != INVALID_DOMAIN_ID) {
    return result;
  }
  if (domainId != this->domainId) {
    return INVALID_DOMAIN_ID;
  }
  switch (uniqueId) {
    case 0:
      parameterWrapper->set(parameters.heaterOn);
      break;
    case 1:
      parameterWrapper->set(parameters.hysteresis);
      break;
    case 2:
      parameterWrapper->set(parameters.heaterSwitchoff);
      break;
    case 3:
      parameterWrapper->set(minTemp);
      break;
    case 4:
      parameterWrapper->set(maxTemp);
      break;
    case 10:
      parameterWrapper->set(parameters.lowerOpLimit);
      break;
    case 11:
      parameterWrapper->set(parameters.upperOpLimit);
      break;
    default:
      return INVALID_IDENTIFIER_ID;
  }
  return returnvalue::OK;
}

#include "fsfw/thermal/ThermalModule.h"

#include "fsfw/monitoring/LimitViolationReporter.h"
#include "fsfw/monitoring/MonitoringMessageContent.h"
#include "fsfw/thermal/AbstractTemperatureSensor.h"

ThermalModule::ThermalModule(gp_id_t moduleTemperaturePoolId, gp_id_t currentStatePoolId,
                             gp_id_t targetStatePoolId, LocalPoolDataSetBase* dataSet,
                             Parameters parameters, RedundantHeater::Parameters heaterParameters)
    : oldStrategy(ACTIVE_SINGLE),
      parameters(parameters),
      moduleTemperature(moduleTemperaturePoolId, dataSet, PoolVariableIF::VAR_WRITE),
      currentState(currentStatePoolId, dataSet, PoolVariableIF::VAR_WRITE),
      targetState(targetStatePoolId, dataSet, PoolVariableIF::VAR_READ) {
  heater = new RedundantHeater(heaterParameters);
}

ThermalModule::ThermalModule(gp_id_t moduleTemperaturePoolId, LocalPoolDataSetBase* dataSet)
    : oldStrategy(ACTIVE_SINGLE),
      parameters({0, 0}),
      moduleTemperature(moduleTemperaturePoolId, dataSet, PoolVariableIF::VAR_WRITE),
      currentState(gp_id_t(), dataSet, PoolVariableIF::VAR_WRITE),
      targetState(gp_id_t(), dataSet, PoolVariableIF::VAR_READ) {}

ThermalModule::~ThermalModule() { delete heater; }

void ThermalModule::performOperation(uint8_t opCode) {
  if (heater != nullptr) {
    heater->performOperation(0);
  }
}

void ThermalModule::performMode(Strategy strategy) {
  calculateTemperature();

  bool safeOnly = (strategy == ACTIVE_SURVIVAL);
  ThermalComponentIF::HeaterRequest componentHeaterRequest =
      letComponentsPerformAndDeciceIfWeNeedToHeat(safeOnly);

  if (heater == nullptr) {
    informComponentsAboutHeaterState(false, NONE);
    return;
  }

  bool heating = calculateModuleHeaterRequestAndSetModuleStatus(strategy);

  if (componentHeaterRequest != ThermalComponentIF::HEATER_DONT_CARE) {
    // Components overwrite the module request.
    heating = ((componentHeaterRequest == ThermalComponentIF::HEATER_REQUEST_ON) or
               (componentHeaterRequest == ThermalComponentIF::HEATER_REQUEST_EMERGENCY_ON));
  }

  bool dual = (strategy == ACTIVE_DUAL);

  if (strategy == PASSIVE) {
    informComponentsAboutHeaterState(false, NONE);
    if (oldStrategy != PASSIVE) {
      heater->set(false, false, true);
    }
  } else {
    if (safeOnly) {
      informComponentsAboutHeaterState(heating, SAFE);
    } else {
      informComponentsAboutHeaterState(heating, ALL);
    }
    heater->set(heating, dual);
  }
  oldStrategy = strategy;
}

float ThermalModule::getTemperature() { return moduleTemperature.value; }

void ThermalModule::registerSensor(AbstractTemperatureSensor* sensor) { sensors.push_back(sensor); }

void ThermalModule::registerComponent(ThermalComponentIF* component,
                                      ThermalComponentIF::Priority priority) {
  components.push_back(ComponentData({component, priority, ThermalComponentIF::HEATER_DONT_CARE}));
}

void ThermalModule::calculateTemperature() {
  uint32_t numberOfValidSensors = 0;
  moduleTemperature = 0;
  std::list<AbstractTemperatureSensor*>::iterator iter = sensors.begin();
  for (; iter != sensors.end(); iter++) {
    if ((*iter)->isValid()) {
      moduleTemperature = moduleTemperature.value + (*iter)->getTemperature();
      numberOfValidSensors++;
    }
  }
  if (numberOfValidSensors != 0) {
    moduleTemperature = moduleTemperature.value / numberOfValidSensors;
    moduleTemperature.setValid(PoolVariableIF::VALID);
  } else {
    moduleTemperature.value = thermal::INVALID_TEMPERATURE;
    moduleTemperature.setValid(PoolVariableIF::INVALID);
  }
}

ThermalComponentIF* ThermalModule::findComponent(object_id_t objectId) {
  std::list<ComponentData>::iterator iter = components.begin();
  for (; iter != components.end(); iter++) {
    if (iter->component->getObjectId() == objectId) {
      return iter->component;
    }
  }
  return NULL;
}

ThermalComponentIF::HeaterRequest ThermalModule::letComponentsPerformAndDeciceIfWeNeedToHeat(
    bool safeOnly) {
  ThermalComponentIF::HeaterRequest heaterRequests[ThermalComponentIF::NUMBER_OF_PRIORITIES];

  survivalTargetTemp = -999;
  targetTemp = -999;

  for (uint8_t i = 0; i < ThermalComponentIF::NUMBER_OF_PRIORITIES; i++) {
    heaterRequests[i] = ThermalComponentIF::HEATER_DONT_CARE;
  }

  std::list<ComponentData>::iterator iter = components.begin();
  for (; iter != components.end(); iter++) {
    updateTargetTemperatures(iter->component, iter->priority == ThermalComponentIF::SAFE);
    ThermalComponentIF::HeaterRequest request = iter->component->performOperation(0);
    iter->request = request;
    if (request != ThermalComponentIF::HEATER_DONT_CARE) {
      if (request < heaterRequests[iter->priority]) {
        heaterRequests[iter->priority] = request;
      }
    }
  }

  if (!safeOnly) {
    for (uint8_t i = ThermalComponentIF::NUMBER_OF_PRIORITIES - 1; i > 0; i--) {
      if (heaterRequests[i - 1] == ThermalComponentIF::HEATER_DONT_CARE) {
        heaterRequests[i - 1] = heaterRequests[i];
      }
    }
  }
  return heaterRequests[0];
}

void ThermalModule::informComponentsAboutHeaterState(bool heaterIsOn, Informee whomToInform) {
  std::list<ComponentData>::iterator iter = components.begin();
  for (; iter != components.end(); iter++) {
    switch (whomToInform) {
      case ALL:
        break;
      case SAFE:
        if (!(iter->priority == ThermalComponentIF::SAFE)) {
          iter->component->markStateIgnored();
          continue;
        }
        break;
      case NONE:
        iter->component->markStateIgnored();
        continue;
    }

    if (heaterIsOn) {
      if ((iter->request == ThermalComponentIF::HEATER_REQUEST_EMERGENCY_OFF) ||
          (iter->request == ThermalComponentIF::HEATER_REQUEST_OFF)) {
        iter->component->markStateIgnored();
      }
    } else {
      if ((iter->request == ThermalComponentIF::HEATER_REQUEST_EMERGENCY_ON) ||
          (iter->request == ThermalComponentIF::HEATER_REQUEST_ON)) {
        iter->component->markStateIgnored();
      }
    }
  }
}

void ThermalModule::initialize(PowerSwitchIF* powerSwitch) {
  if (heater != NULL) {
    heater->setPowerSwitcher(powerSwitch);
  }

  std::list<ComponentData>::iterator iter = components.begin();
  for (; iter != components.end(); iter++) {
    float componentLowerOpLimit = iter->component->getLowerOpLimit();
    if (iter->priority == ThermalComponentIF::SAFE) {
      if (componentLowerOpLimit > survivalTargetTemp) {
        survivalTargetTemp = componentLowerOpLimit;
      }
    } else {
      if (componentLowerOpLimit > targetTemp) {
        targetTemp = componentLowerOpLimit;
      }
    }
  }
  if (survivalTargetTemp > targetTemp) {
    targetTemp = survivalTargetTemp;
  }
}

bool ThermalModule::calculateModuleHeaterRequestAndSetModuleStatus(Strategy strategy) {
  currentState.setValid(PoolVariableIF::VALID);
  if (moduleTemperature == thermal::INVALID_TEMPERATURE) {
    currentState = UNKNOWN;
    return false;
  }

  float limit = targetTemp;
  bool heaterRequest = false;
  if (strategy == ACTIVE_SURVIVAL) {
    limit = survivalTargetTemp;
  }

  if (moduleTemperature.value >= limit) {
    currentState = OPERATIONAL;
  } else {
    currentState = NON_OPERATIONAL;
  }

  limit += parameters.heaterOn;

  if (heating) {
    limit += parameters.hysteresis;
  }

  if (targetState == STATE_REQUEST_HEATING) {
    if (moduleTemperature < limit) {
      heaterRequest = true;
    } else {
      heaterRequest = false;
    }
  }

  heating = heaterRequest;

  return heaterRequest;
}

void ThermalModule::setHeating(bool on) {
  ReturnValue_t result = targetState.read();
  if (result == returnvalue::OK) {
    if (on) {
      targetState.value = STATE_REQUEST_HEATING;
    } else {
      targetState.value = STATE_REQUEST_PASSIVE;
    }
  }
  targetState.setValid(true);
}

void ThermalModule::updateTargetTemperatures(ThermalComponentIF* component, bool isSafe) {
  if (isSafe) {
    if (component->getLowerOpLimit() > survivalTargetTemp) {
      survivalTargetTemp = component->getLowerOpLimit();
    }
  } else {
    if (component->getLowerOpLimit() > targetTemp) {
      targetTemp = component->getLowerOpLimit();
    }
  }
}

void ThermalModule::setOutputInvalid() {
  moduleTemperature = thermal::INVALID_TEMPERATURE;
  moduleTemperature.setValid(PoolVariableIF::INVALID);
  currentState.setValid(PoolVariableIF::INVALID);
  std::list<ComponentData>::iterator iter = components.begin();
  for (; iter != components.end(); iter++) {
    iter->component->setOutputInvalid();
  }
  if (heater != NULL) {
    heater->set(false, true);
  }
}

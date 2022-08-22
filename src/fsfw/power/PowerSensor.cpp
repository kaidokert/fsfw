#include "fsfw/power/PowerSensor.h"

#include "fsfw/ipc/QueueFactory.h"

PowerSensor::PowerSensor(object_id_t objectId, sid_t setId, VariableIds ids, DefaultLimits limits,
                         SensorEvents events, uint16_t confirmationCount)
    : SystemObject(objectId),
      parameterHelper(this),
      healthHelper(this, objectId),
      powerSensorSet(setId),
      current(ids.pidCurrent, &powerSensorSet),
      voltage(ids.pidVoltage, &powerSensorSet),
      power(ids.poolIdPower, &powerSensorSet, PoolVariableIF::VAR_WRITE),
      currentLimit(objectId, MODULE_ID_CURRENT, ids.pidCurrent, confirmationCount,
                   limits.currentMin, limits.currentMax, events.currentLow, events.currentHigh),
      voltageLimit(objectId, MODULE_ID_VOLTAGE, ids.pidVoltage, confirmationCount,
                   limits.voltageMin, limits.voltageMax, events.voltageLow, events.voltageHigh) {
  commandQueue = QueueFactory::instance()->createMessageQueue();
}

PowerSensor::~PowerSensor() { QueueFactory::instance()->deleteMessageQueue(commandQueue); }

ReturnValue_t PowerSensor::calculatePower() {
  powerSensorSet.read();
  ReturnValue_t result1 = returnvalue::FAILED;
  ReturnValue_t result2 = returnvalue::FAILED;
  if (healthHelper.healthTable->isHealthy(getObjectId()) && voltage.isValid() &&
      current.isValid()) {
    result1 = voltageLimit.doCheck(voltage.value);
    result2 = currentLimit.doCheck(current.value);
  } else {
    voltageLimit.setToInvalid();
    currentLimit.setToInvalid();
    result1 = OBJECT_NOT_HEALTHY;
  }
  if (result1 != returnvalue::OK || result2 != returnvalue::OK) {
    result1 = MonitoringIF::INVALID;
    power.setValid(PoolVariableIF::INVALID);
  } else {
    power.setValid(PoolVariableIF::VALID);
    power.value = current.value * voltage.value;
  }
  powerSensorSet.commit();
  return result1;
}

ReturnValue_t PowerSensor::performOperation(uint8_t opCode) {
  checkCommandQueue();
  return returnvalue::OK;
}

MessageQueueId_t PowerSensor::getCommandQueue() const { return commandQueue->getId(); }

ReturnValue_t PowerSensor::initialize() {
  ReturnValue_t result = SystemObject::initialize();
  if (result != returnvalue::OK) {
    return result;
  }
  result = healthHelper.initialize();
  if (result != returnvalue::OK) {
    return result;
  }
  result = parameterHelper.initialize();
  if (result != returnvalue::OK) {
    return result;
  }
  return result;
}

void PowerSensor::setAllMonitorsToUnchecked() {
  currentLimit.setToUnchecked();
  voltageLimit.setToUnchecked();
}

void PowerSensor::checkCommandQueue() {
  CommandMessage command;
  ReturnValue_t result = commandQueue->receiveMessage(&command);
  if (result != returnvalue::OK) {
    return;
  }
  result = healthHelper.handleHealthCommand(&command);
  if (result == returnvalue::OK) {
    return;
  }
  result = parameterHelper.handleParameterMessage(&command);
  if (result == returnvalue::OK) {
    return;
  }
  command.setToUnknownCommand();
  commandQueue->reply(&command);
}

void PowerSensor::setDataPoolEntriesInvalid() {
  powerSensorSet.read();
  powerSensorSet.setValidity(false, true);
  powerSensorSet.commit();
}

float PowerSensor::getPower() {
  if (power.isValid()) {
    return power.value;
  } else {
    return 0.0;
  }
}

ReturnValue_t PowerSensor::setHealth(HealthState health) {
  healthHelper.setHealth(health);
  return returnvalue::OK;
}

HasHealthIF::HealthState PowerSensor::getHealth() { return healthHelper.getHealth(); }

ReturnValue_t PowerSensor::getParameter(uint8_t domainId, uint8_t uniqueId,
                                        ParameterWrapper* parameterWrapper,
                                        const ParameterWrapper* newValues, uint16_t startAtIndex) {
  ReturnValue_t result =
      currentLimit.getParameter(domainId, uniqueId, parameterWrapper, newValues, startAtIndex);
  if (result != INVALID_DOMAIN_ID) {
    return result;
  }
  result = voltageLimit.getParameter(domainId, uniqueId, parameterWrapper, newValues, startAtIndex);
  return result;
}

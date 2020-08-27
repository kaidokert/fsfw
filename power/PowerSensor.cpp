#include "PowerSensor.h"
#include "../ipc/QueueFactory.h"

PowerSensor::PowerSensor(object_id_t setId, VariableIds ids,
		DefaultLimits limits, SensorEvents events, uint16_t confirmationCount) :
		SystemObject(setId), commandQueue(NULL), parameterHelper(this), healthHelper(this, setId), set(), current(
				ids.pidCurrent, &set), voltage(ids.pidVoltage, &set), power(
				ids.poolIdPower, &set, PoolVariableIF::VAR_WRITE), currentLimit(
				setId, MODULE_ID_CURRENT, ids.pidCurrent, confirmationCount,
				limits.currentMin, limits.currentMax, events.currentLow,
				events.currentHigh), voltageLimit(setId, MODULE_ID_VOLTAGE,
				ids.pidVoltage, confirmationCount, limits.voltageMin,
				limits.voltageMax, events.voltageLow, events.voltageHigh) {
		commandQueue = QueueFactory::instance()->createMessageQueue();
}

PowerSensor::~PowerSensor() {
	QueueFactory::instance()->deleteMessageQueue(commandQueue);
}

ReturnValue_t PowerSensor::calculatePower() {
	set.read();
	ReturnValue_t result1 = HasReturnvaluesIF::RETURN_FAILED;
	ReturnValue_t result2 = HasReturnvaluesIF::RETURN_FAILED;
	if (healthHelper.healthTable->isHealthy(getObjectId()) && voltage.isValid()
			&& current.isValid()) {
		result1 = voltageLimit.doCheck(voltage);
		result2 = currentLimit.doCheck(current);
	} else {
		voltageLimit.setToInvalid();
		currentLimit.setToInvalid();
		result1 = OBJECT_NOT_HEALTHY;
	}
	if (result1 != HasReturnvaluesIF::RETURN_OK
			|| result2 != HasReturnvaluesIF::RETURN_OK) {
		result1 = MonitoringIF::INVALID;
		power.setValid(PoolVariableIF::INVALID);
	} else {
		power.setValid(PoolVariableIF::VALID);
		power = current * voltage;
	}
	set.commit();
	return result1;
}

ReturnValue_t PowerSensor::performOperation(uint8_t opCode) {
	checkCommandQueue();
	return HasReturnvaluesIF::RETURN_OK;
}

MessageQueueId_t PowerSensor::getCommandQueue() const {
	return commandQueue->getId();
}

ReturnValue_t PowerSensor::initialize() {
	ReturnValue_t result = SystemObject::initialize();
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	result = healthHelper.initialize();
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	result = parameterHelper.initialize();
	if (result != HasReturnvaluesIF::RETURN_OK) {
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
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return;
	}
	result = healthHelper.handleHealthCommand(&command);
	if (result == HasReturnvaluesIF::RETURN_OK) {
		return;
	}
	result = parameterHelper.handleParameterMessage(&command);
	if (result == HasReturnvaluesIF::RETURN_OK) {
		return;
	}
	command.setToUnknownCommand();
	commandQueue->reply(&command);
}

void PowerSensor::setDataPoolEntriesInvalid() {
	set.read();
	set.commit(PoolVariableIF::INVALID);
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
	return HasReturnvaluesIF::RETURN_OK;
}

HasHealthIF::HealthState PowerSensor::getHealth() {
	return healthHelper.getHealth();
}

ReturnValue_t PowerSensor::getParameter(uint8_t domainId, uint16_t parameterId,
		ParameterWrapper* parameterWrapper, const ParameterWrapper* newValues,
		uint16_t startAtIndex) {
	ReturnValue_t result = currentLimit.getParameter(domainId, parameterId,
			parameterWrapper, newValues, startAtIndex);
	if (result != INVALID_DOMAIN_ID) {
		return result;
	}
	result = voltageLimit.getParameter(domainId, parameterId, parameterWrapper,
			newValues, startAtIndex);
	return result;
}

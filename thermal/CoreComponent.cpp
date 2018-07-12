#include "CoreComponent.h"

CoreComponent::CoreComponent(object_id_t reportingObjectId, uint8_t domainId,
		uint32_t temperaturePoolId, uint32_t targetStatePoolId,
		uint32_t currentStatePoolId, uint32_t requestPoolId, DataSet* dataSet,
		AbstractTemperatureSensor* sensor,
		AbstractTemperatureSensor* firstRedundantSensor,
		AbstractTemperatureSensor* secondRedundantSensor,
		ThermalModuleIF* thermalModule, Parameters parameters,
		Priority priority, StateRequest initialTargetState) :
		sensor(sensor), firstRedundantSensor(firstRedundantSensor), secondRedundantSensor(
				secondRedundantSensor), thermalModule(thermalModule), temperature(
				temperaturePoolId, dataSet, PoolVariableIF::VAR_WRITE), targetState(
				targetStatePoolId, dataSet, PoolVariableIF::VAR_READ), currentState(
				currentStatePoolId, dataSet, PoolVariableIF::VAR_WRITE), heaterRequest(
				requestPoolId, dataSet, PoolVariableIF::VAR_WRITE), isHeating(
		false), isSafeComponent(priority == SAFE), minTemp(999), maxTemp(
				AbstractTemperatureSensor::ZERO_KELVIN_C), parameters(
				parameters), temperatureMonitor(reportingObjectId,
				domainId + 1,
				DataPool::poolIdAndPositionToPid(temperaturePoolId, 0),
				COMPONENT_TEMP_CONFIRMATION), domainId(domainId) {
	if (thermalModule != NULL) {
		thermalModule->registerComponent(this, priority);
	}
	//Set thermal state once, then leave to operator.
	DataSet mySet;
	PoolVariable<int8_t> writableTargetState(targetStatePoolId, &mySet,
			PoolVariableIF::VAR_WRITE);
	writableTargetState = initialTargetState;
	mySet.commit(PoolVariableIF::VALID);
}

CoreComponent::~CoreComponent() {
}

ThermalComponentIF::HeaterRequest CoreComponent::performOperation(uint8_t opCode) {
	HeaterRequest request = HEATER_DONT_CARE;
	//SHOULDDO: Better pass db_float_t* to getTemperature and set it invalid if invalid.
	temperature = getTemperature();
	updateMinMaxTemp();
	if ((temperature != INVALID_TEMPERATURE)) {
		temperature.setValid(PoolVariableIF::VALID);
		State state = getState(temperature, getParameters(), targetState);
		currentState = state;
		checkLimits(state);
		request = getHeaterRequest(targetState, temperature, getParameters());
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

void CoreComponent::markStateIgnored() {
	currentState = getIgnoredState(currentState);
}

object_id_t CoreComponent::getObjectId() {
	return temperatureMonitor.getReporterId();
}

float CoreComponent::getLowerOpLimit() {
	return parameters.lowerOpLimit;
}

ReturnValue_t CoreComponent::setTargetState(int8_t newState) {
	DataSet mySet;
	PoolVariable<int8_t> writableTargetState(targetState.getDataPoolId(),
			&mySet, PoolVariableIF::VAR_READ_WRITE);
	mySet.read();
	if ((writableTargetState == STATE_REQUEST_OPERATIONAL)
			&& (newState != STATE_REQUEST_IGNORE)) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	switch (newState) {
	case STATE_REQUEST_HEATING:
	case STATE_REQUEST_IGNORE:
	case STATE_REQUEST_OPERATIONAL:
		writableTargetState = newState;
		break;
	case STATE_REQUEST_NON_OPERATIONAL:
	default:
		return INVALID_TARGET_STATE;
	}
	mySet.commit(PoolVariableIF::VALID);
	return HasReturnvaluesIF::RETURN_OK;
}

void CoreComponent::setOutputInvalid() {
	temperature = INVALID_TEMPERATURE;
	temperature.setValid(PoolVariableIF::INVALID);
	currentState.setValid(PoolVariableIF::INVALID);
	heaterRequest = HEATER_DONT_CARE;
	heaterRequest.setValid(PoolVariableIF::INVALID);
	temperatureMonitor.setToUnchecked();
}

float CoreComponent::getTemperature() {
	if ((sensor != NULL) && (sensor->isValid())) {
		return sensor->getTemperature();
	}

	if ((firstRedundantSensor != NULL) && (firstRedundantSensor->isValid())) {
		return firstRedundantSensor->getTemperature();
	}

	if ((secondRedundantSensor != NULL) && (secondRedundantSensor->isValid())) {
		return secondRedundantSensor->getTemperature();
	}

	if (thermalModule != NULL) {
		float temperature = thermalModule->getTemperature();
		if (temperature != ThermalModuleIF::INVALID_TEMPERATURE) {
			return temperature;
		} else {
			return INVALID_TEMPERATURE;
		}
	} else {
		return INVALID_TEMPERATURE;
	}
}

ThermalComponentIF::State CoreComponent::getState(float temperature,
		Parameters parameters, int8_t targetState) {
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

void CoreComponent::checkLimits(ThermalComponentIF::State state) {
	//Checks operational limits only.
	temperatureMonitor.translateState(state, temperature.value,
			getParameters().lowerOpLimit, getParameters().upperOpLimit);

}

ThermalComponentIF::HeaterRequest CoreComponent::getHeaterRequest(
		int8_t targetState, float temperature, Parameters parameters) {
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

ThermalComponentIF::State CoreComponent::getIgnoredState(int8_t state) {
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

void CoreComponent::updateMinMaxTemp() {
	if (temperature == INVALID_TEMPERATURE) {
		return;
	}
	if (temperature < minTemp) {
		minTemp = temperature;
	}
	if (temperature > maxTemp) {
		maxTemp = temperature;
	}
}

uint8_t CoreComponent::getDomainId() const {
	return domainId;
}

CoreComponent::Parameters CoreComponent::getParameters() {
	return parameters;
}

ReturnValue_t CoreComponent::getParameter(uint8_t domainId,
		uint16_t parameterId, ParameterWrapper* parameterWrapper,
		const ParameterWrapper* newValues, uint16_t startAtIndex) {
	ReturnValue_t result = temperatureMonitor.getParameter(domainId,
			parameterId, parameterWrapper, newValues, startAtIndex);
	if (result != INVALID_DOMAIN_ID) {
		return result;
	}
	if (domainId != this->domainId) {
		return INVALID_DOMAIN_ID;
	}
	switch (parameterId) {
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
		return INVALID_MATRIX_ID;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

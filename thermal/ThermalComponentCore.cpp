#include "ThermalComponentCore.h"

ThermalComponentCore::ThermalComponentCore(object_id_t reportingObjectId,
        uint8_t domainId, gp_id_t temperaturePoolId,
        gp_id_t targetStatePoolId, gp_id_t currentStatePoolId,
		gp_id_t requestPoolId, LocalDataSetBase* dataSet, Parameters parameters,
		StateRequest initialTargetState) :
		temperature(temperaturePoolId, dataSet, PoolVariableIF::VAR_WRITE),
		targetState(targetStatePoolId, dataSet, PoolVariableIF::VAR_READ),
		currentState(currentStatePoolId, dataSet, PoolVariableIF::VAR_WRITE),
		heaterRequest(requestPoolId, dataSet, PoolVariableIF::VAR_WRITE),
		parameters(parameters),
		temperatureMonitor(reportingObjectId, domainId + 1,
		        GlobalDataPool::poolIdAndPositionToPid(temperaturePoolId, 0),
				COMPONENT_TEMP_CONFIRMATION), domainId(domainId) {
	//Set thermal state once, then leave to operator.
	GlobDataSet mySet;
	gp_uint8_t writableTargetState(targetStatePoolId, &mySet,
			PoolVariableIF::VAR_WRITE);
	writableTargetState = initialTargetState;
	mySet.commit(PoolVariableIF::VALID);
}

void ThermalComponentCore::addSensor(AbstractTemperatureSensor* sensor) {
    this->sensor = sensor;
}

void ThermalComponentCore::addFirstRedundantSensor(
        AbstractTemperatureSensor *firstRedundantSensor) {
    this->firstRedundantSensor = firstRedundantSensor;
}

void ThermalComponentCore::addSecondRedundantSensor(
        AbstractTemperatureSensor *secondRedundantSensor) {
    this->secondRedundantSensor = secondRedundantSensor;
}

void ThermalComponentCore::addThermalModule(ThermalModule *thermalModule,
        Priority priority) {
    this->thermalModule = thermalModule;
    if(thermalModule != nullptr) {
        thermalModule->registerComponent(this, priority);
    }
}

void ThermalComponentCore::setPriority(Priority priority) {
    if(priority == SAFE) {
        this->isSafeComponent = true;
    }
}

ThermalComponentCore::~ThermalComponentCore() {
}

ThermalComponentIF::HeaterRequest ThermalComponentCore::performOperation(
        uint8_t opCode) {
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

void ThermalComponentCore::markStateIgnored() {
	currentState = getIgnoredState(currentState);
}

object_id_t ThermalComponentCore::getObjectId() {
	return temperatureMonitor.getReporterId();
}

float ThermalComponentCore::getLowerOpLimit() {
	return parameters.lowerOpLimit;
}



ReturnValue_t ThermalComponentCore::setTargetState(int8_t newState) {
	GlobDataSet mySet;
	gp_uint8_t writableTargetState(targetState.getDataPoolId(),
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

void ThermalComponentCore::setOutputInvalid() {
	temperature = INVALID_TEMPERATURE;
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

	if ((firstRedundantSensor != nullptr) &&
	        (firstRedundantSensor->isValid())) {
		return firstRedundantSensor->getTemperature();
	}

	if ((secondRedundantSensor != nullptr) &&
	        (secondRedundantSensor->isValid())) {
		return secondRedundantSensor->getTemperature();
	}

	if (thermalModule != nullptr) {
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

ThermalComponentIF::State ThermalComponentCore::getState(float temperature,
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

void ThermalComponentCore::checkLimits(ThermalComponentIF::State state) {
	//Checks operational limits only.
	temperatureMonitor.translateState(state, temperature.value,
			getParameters().lowerOpLimit, getParameters().upperOpLimit);

}

ThermalComponentIF::HeaterRequest ThermalComponentCore::getHeaterRequest(
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

uint8_t ThermalComponentCore::getDomainId() const {
	return domainId;
}

ThermalComponentCore::Parameters ThermalComponentCore::getParameters() {
	return parameters;
}

ReturnValue_t ThermalComponentCore::getParameter(uint8_t domainId,
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
		return INVALID_IDENTIFIER_ID;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

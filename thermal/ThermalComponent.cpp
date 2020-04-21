#include "ThermalComponent.h"

ThermalComponent::ThermalComponent(object_id_t reportingObjectId,
		uint8_t domainId, uint32_t temperaturePoolId,
		uint32_t targetStatePoolId, uint32_t currentStatePoolId,
		uint32_t requestPoolId, DataSet* dataSet,
		AbstractTemperatureSensor* sensor,
		AbstractTemperatureSensor* firstRedundantSensor,
		AbstractTemperatureSensor* secondRedundantSensor,
		ThermalModuleIF* thermalModule, Parameters parameters,
		Priority priority) :
		CoreComponent(reportingObjectId, domainId, temperaturePoolId,
				targetStatePoolId, currentStatePoolId, requestPoolId, dataSet,
				sensor, firstRedundantSensor, secondRedundantSensor,
				thermalModule,
				{ parameters.lowerOpLimit, parameters.upperOpLimit,
						parameters.heaterOn, parameters.hysteresis,
						parameters.heaterSwitchoff }, priority,
				ThermalComponentIF::STATE_REQUEST_NON_OPERATIONAL), nopParameters(
				{ parameters.lowerNopLimit, parameters.upperNopLimit }) {
}

ThermalComponent::~ThermalComponent() {
}

ReturnValue_t ThermalComponent::setTargetState(int8_t newState) {
	DataSet mySet;
	PoolVariable<int8_t> writableTargetState(targetState.getDataPoolId(),
			&mySet, PoolVariableIF::VAR_READ_WRITE);
	mySet.read();
	if ((writableTargetState == STATE_REQUEST_OPERATIONAL)
			&& (newState != STATE_REQUEST_IGNORE)) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	switch (newState) {
	case STATE_REQUEST_NON_OPERATIONAL:
		writableTargetState = newState;
		mySet.commit(PoolVariableIF::VALID);
		return HasReturnvaluesIF::RETURN_OK;
	default:
		return CoreComponent::setTargetState(newState);
	}
}

ReturnValue_t ThermalComponent::setLimits(const uint8_t* data, uint32_t size) {
	if (size != 4 * sizeof(parameters.lowerOpLimit)) {
		return MonitoringIF::INVALID_SIZE;
	}
	size_t readSize = size;
	SerializeAdapter::deSerialize(&nopParameters.lowerNopLimit, &data,
			&readSize, SerializeIF::Endianness::BIG);
	SerializeAdapter::deSerialize(&parameters.lowerOpLimit, &data,
			&readSize, SerializeIF::Endianness::BIG);
	SerializeAdapter::deSerialize(&parameters.upperOpLimit, &data,
			&readSize, SerializeIF::Endianness::BIG);
	SerializeAdapter::deSerialize(&nopParameters.upperNopLimit, &data,
			&readSize, SerializeIF::Endianness::BIG);
	return HasReturnvaluesIF::RETURN_OK;
}

ThermalComponentIF::State ThermalComponent::getState(float temperature,
		CoreComponent::Parameters parameters, int8_t targetState) {
	if (temperature < nopParameters.lowerNopLimit) {
		return OUT_OF_RANGE_LOW;
	} else {
		State state = CoreComponent::getState(temperature, parameters,
				targetState);
		if (state != NON_OPERATIONAL_HIGH
				&& state != NON_OPERATIONAL_HIGH_IGNORED) {
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
	if (targetState == STATE_REQUEST_OPERATIONAL || targetState == STATE_REQUEST_IGNORE) {
		CoreComponent::checkLimits(state);
		return;
	}
	//If component is not operational, it checks the NOP limits.
	temperatureMonitor.translateState(state, temperature.value,
			nopParameters.lowerNopLimit, nopParameters.upperNopLimit, false);
}

ThermalComponentIF::HeaterRequest ThermalComponent::getHeaterRequest(
		int8_t targetState, float temperature,
		CoreComponent::Parameters parameters) {
	if (targetState == STATE_REQUEST_IGNORE) {
		isHeating = false;
		return HEATER_DONT_CARE;
	}

	if (temperature
			> nopParameters.upperNopLimit - parameters.heaterSwitchoff) {
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

	if ((targetState == STATE_REQUEST_OPERATIONAL)
			|| (targetState == STATE_REQUEST_HEATING)) {
		if (temperature < opHeaterLimit) {
			isHeating = true;
			return HEATER_REQUEST_ON;
		}
		if (temperature
				> parameters.upperOpLimit - parameters.heaterSwitchoff) {
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
		return CoreComponent::getIgnoredState(state);
	}
}

ReturnValue_t ThermalComponent::getParameter(uint8_t domainId,
		uint16_t parameterId, ParameterWrapper* parameterWrapper,
		const ParameterWrapper* newValues, uint16_t startAtIndex) {
	ReturnValue_t result = CoreComponent::getParameter(domainId, parameterId,
			parameterWrapper, newValues, startAtIndex);
	if (result != INVALID_MATRIX_ID) {
		return result;
	}
	switch (parameterId) {
	case 12:
		parameterWrapper->set(nopParameters.lowerNopLimit);
		break;
	case 13:
		parameterWrapper->set(nopParameters.upperNopLimit);
		break;
	default:
		return INVALID_MATRIX_ID;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

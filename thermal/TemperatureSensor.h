#ifndef TEMPERATURESENSOR_H_
#define TEMPERATURESENSOR_H_

#include "../datapool/DataSet.h"
#include "AbstractTemperatureSensor.h"
#include "../monitoring/LimitMonitor.h"

template<typename T>
class TemperatureSensor: public AbstractTemperatureSensor {
public:
	struct Parameters {
		float a;
		float b;
		float c;
		T lowerLimit;
		T upperLimit;
		float gradient;
	};
	struct UsedParameters {
		UsedParameters(Parameters parameters) :
				a(parameters.a), b(parameters.b), c(parameters.c), gradient(
						parameters.gradient) {
		}
		float a;
		float b;
		float c;
		float gradient;
	};

	static const uint16_t ADDRESS_A = 0;
	static const uint16_t ADDRESS_B = 1;
	static const uint16_t ADDRESS_C = 2;
	static const uint16_t ADDRESS_GRADIENT = 3;

	static const uint16_t DEFAULT_CONFIRMATION_COUNT = 1; //!< Changed due to issue with later temperature checking even tough the sensor monitor was confirming already (Was 10 before with comment = Correlates to a 10s confirmation time. Chosen rather large, should not be so bad for components and helps survive glitches.)

	static const uint8_t DOMAIN_ID_SENSOR = 1;
private:
	void setInvalid() {
		outputTemperature = INVALID_TEMPERATURE;
		outputTemperature.setValid(false);
		uptimeOfOldTemperature.tv_sec = INVALID_UPTIME;
		sensorMonitor.setToInvalid();
	}
protected:
	static const int32_t INVALID_UPTIME = 0;

	UsedParameters parameters;

	T *inputTemperature;

	PoolVariableIF *poolVariable;

	PoolVariable<float> outputTemperature;

	LimitMonitor<T> sensorMonitor;

	float oldTemperature;
	timeval uptimeOfOldTemperature;

	virtual float calculateOutputTemperature(T inputTemperature) {
		return parameters.a * inputTemperature * inputTemperature
				+ parameters.b * inputTemperature + parameters.c;
	}

	void doChildOperation() {
		if (!poolVariable->isValid()
				|| !healthHelper.healthTable->isHealthy(getObjectId())) {
			setInvalid();
			return;
		}

		outputTemperature = calculateOutputTemperature(*inputTemperature);
		outputTemperature.setValid(PoolVariableIF::VALID);

		timeval uptime;
		Clock::getUptime(&uptime);

		if (uptimeOfOldTemperature.tv_sec != INVALID_UPTIME) {
			//In theory, we could use an AbsValueMonitor to monitor the gradient.
			//But this would require storing the gradient in DP and quite some overhead.
			//The concept of delta limits is a bit strange anyway.
			float deltaTime;
			float deltaTemp;

			deltaTime = (uptime.tv_sec + uptime.tv_usec / 1000000.)
					- (uptimeOfOldTemperature.tv_sec
							+ uptimeOfOldTemperature.tv_usec / 1000000.);
			deltaTemp = oldTemperature - outputTemperature;
			if (deltaTemp < 0) {
				deltaTemp = -deltaTemp;
			}
			if (parameters.gradient < deltaTemp / deltaTime) {
				triggerEvent(TEMP_SENSOR_GRADIENT);
				//Don't set invalid, as we did not recognize it as invalid with full authority, let FDIR handle it
			}
		}

		//Check is done against raw limits. SHOULDDO: Why? Using °C would be more easy to handle.
		sensorMonitor.doCheck(*inputTemperature);

		if (sensorMonitor.isOutOfLimits()) {
			uptimeOfOldTemperature.tv_sec = INVALID_UPTIME;
			outputTemperature.setValid(PoolVariableIF::INVALID);
			outputTemperature = INVALID_TEMPERATURE;
		} else {
			oldTemperature = outputTemperature;
			uptimeOfOldTemperature = uptime;
		}
	}

public:
	TemperatureSensor(object_id_t setObjectid,
			T *inputTemperature, PoolVariableIF *poolVariable,
			uint8_t vectorIndex, Parameters parameters, uint32_t datapoolId,
			DataSet *outputSet, ThermalModuleIF *thermalModule) :
			AbstractTemperatureSensor(setObjectid, thermalModule), parameters(
					parameters), inputTemperature(inputTemperature), poolVariable(
					poolVariable), outputTemperature(datapoolId, outputSet,
					PoolVariableIF::VAR_WRITE), sensorMonitor(setObjectid,
							DOMAIN_ID_SENSOR,
					DataPool::poolIdAndPositionToPid(
							poolVariable->getDataPoolId(), vectorIndex),
					DEFAULT_CONFIRMATION_COUNT, parameters.lowerLimit,
					parameters.upperLimit, TEMP_SENSOR_LOW, TEMP_SENSOR_HIGH), oldTemperature(
					20), uptimeOfOldTemperature( { INVALID_TEMPERATURE, 0 }) {

	}

	float getTemperature() {
		return outputTemperature;
	}

	bool isValid() {
		return outputTemperature.isValid();
	}

	virtual ReturnValue_t getParameter(uint8_t domainId, uint16_t parameterId,
			ParameterWrapper *parameterWrapper,
			const ParameterWrapper *newValues, uint16_t startAtIndex) {
		ReturnValue_t result = sensorMonitor.getParameter(domainId, parameterId,
				parameterWrapper, newValues, startAtIndex);
		if (result != INVALID_DOMAIN_ID) {
			return result;
		}
		if (domainId != this->DOMAIN_ID_BASE) {
			return INVALID_DOMAIN_ID;
		}
		switch (parameterId) {
		case ADDRESS_A:
			parameterWrapper->set(parameters.a);
			break;
		case ADDRESS_B:
			parameterWrapper->set(parameters.b);
			break;
		case ADDRESS_C:
			parameterWrapper->set(parameters.c);
			break;
		case ADDRESS_GRADIENT:
			parameterWrapper->set(parameters.gradient);
			break;
		default:
			return INVALID_MATRIX_ID;
		}
		return HasReturnvaluesIF::RETURN_OK;
	}

	virtual void resetOldState() {
		sensorMonitor.setToUnchecked();
	}

};

#endif /* TEMPERATURESENSOR_H_ */

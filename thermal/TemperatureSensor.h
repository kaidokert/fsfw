#ifndef TEMPERATURESENSOR_H_
#define TEMPERATURESENSOR_H_

#include <framework/thermal/AbstractTemperatureSensor.h>
#include <framework/datapool/DataSet.h>
#include <framework/monitoring/LimitMonitor.h>

/**
 * @brief 	This building block handles non-linear value conversion and
 * 			range checks for analog temperature sensors.
 * @details This class can be used to perform all necessary tasks for temperature sensors.
 *          A sensor can be instantiated by calling the constructor.
 *          The temperature is calculated from an input value with
 *			the calculateOutputTemperature() function. Range checking and
 *			limit monitoring is performed automatically.
 *
 * @ingroup thermal
 */

template<typename T>
class TemperatureSensor: public AbstractTemperatureSensor {
public:
	/**
	 * This structure contains parameters required for range checking
	 * and the conversion from the input value to the output temperature.
	 * a, b and c can be any parameters required to calculate the output
	 * temperature from the input value, for example parameters for the
	 * Callendar-Van-Dusen equation(see datasheet of used sensor / ADC)
	 *
	 * The parameters a,b and c are used in the calculateOutputTemperature() call.
	 *
	 * The lower and upper limits can be specified in °C or in the input value
	 * format
	 */
	struct Parameters {
		float a;
		float b;
		float c;
		float maxGradient;
	};

	/**
	 * Forward declaration for explicit instantiation of used parameters.
	 */
	struct UsedParameters {
		UsedParameters(Parameters parameters) :
			a(parameters.a), b(parameters.b), c(parameters.c),
			gradient(parameters.maxGradient) {}
		float a;
		float b;
		float c;
		float gradient;
	};

	/**
	 * Constructor to check against raw input values
	 * @param setObjectid objectId of the sensor object
	 * @param inputValue Input value which is converted to a temperature
	 * @param poolVariable Pool Variable to store the temperature value
	 * @param vectorIndex Vector Index for the sensor monitor
	 * @param parameters Calculation parameters, temperature limits, gradient limit
	 * @param datapoolId Datapool ID of the output temperature
	 * @param outputSet Output dataset for the output temperature to fetch it with read()
	 * @param thermalModule respective thermal module, if it has one
	 */
	TemperatureSensor(object_id_t setObjectid,
			T *inputValue, T lowerLimit, T upperLimit, PoolVariableIF *poolVariable,
			uint8_t vectorIndex, uint32_t datapoolId, Parameters parameters = {0, 0, 0, 0},
			DataSet *outputSet = NULL, ThermalModuleIF *thermalModule = NULL) :
			AbstractTemperatureSensor(setObjectid, thermalModule), parameters(parameters),
			inputValue(inputValue), poolVariable(poolVariable),
			outputTemperature(datapoolId, outputSet, PoolVariableIF::VAR_WRITE),
			oldTemperature(20), uptimeOfOldTemperature( { INVALID_TEMPERATURE, 0 })
	{
		sensorMonitorRaw = new LimitMonitor<T>(setObjectid, DOMAIN_ID_SENSOR,
				DataPool::poolIdAndPositionToPid(poolVariable->getDataPoolId(), vectorIndex),
				DEFAULT_CONFIRMATION_COUNT, lowerLimit, upperLimit,
				TEMP_SENSOR_LOW, TEMP_SENSOR_HIGH);
		delete sensorMonitor;
	}

	/**
	 * Constructor do check against °C values
	 */
	TemperatureSensor(object_id_t setObjectid,
		T *inputValue, float lowerLimit, float upperLimit, PoolVariableIF *poolVariable,
		uint8_t vectorIndex, uint32_t datapoolId, Parameters parameters = {0, 0, 0, 0},
		DataSet *outputSet = NULL, ThermalModuleIF *thermalModule = NULL) :
		AbstractTemperatureSensor(setObjectid, thermalModule), parameters(parameters),
		inputValue(inputValue), poolVariable(poolVariable),
		outputTemperature(datapoolId, outputSet, PoolVariableIF::VAR_WRITE),
		oldTemperature(20), uptimeOfOldTemperature( { INVALID_TEMPERATURE, 0 })
	{
		sensorMonitor = new LimitMonitor<float>(setObjectid, DOMAIN_ID_SENSOR,
				DataPool::poolIdAndPositionToPid(poolVariable->getDataPoolId(), vectorIndex),
				DEFAULT_CONFIRMATION_COUNT, lowerLimit, upperLimit,
				TEMP_SENSOR_LOW, TEMP_SENSOR_HIGH);
		delete sensorMonitorRaw;
	}

protected:
	/**
	 * This formula is used to calculate the temperature from an input value
	 * with an arbitrary type.
	 * A default implementation is provided but can be replaced depending
	 * on the required calculation.
	 * @param inputTemperature
	 * @return
	 */
	virtual float calculateOutputTemperature(T inputValue) {
		return parameters.a * inputValue * inputValue
				+ parameters.b * inputValue + parameters.c;
	}


private:
	void setInvalid() {
		outputTemperature = INVALID_TEMPERATURE;
		outputTemperature.setValid(false);
		uptimeOfOldTemperature.tv_sec = INVALID_UPTIME;
		sensorMonitor->setToInvalid();
	}
protected:
	static const int32_t INVALID_UPTIME = 0;

	UsedParameters parameters;

	T * inputValue;

	PoolVariableIF *poolVariable;

	PoolVariable<float> outputTemperature;

	LimitMonitor<T> * sensorMonitorRaw;
	LimitMonitor<float> * sensorMonitor;

	float oldTemperature;
	timeval uptimeOfOldTemperature;

	void doChildOperation() {
		if (!poolVariable->isValid()
				|| !healthHelper.healthTable->isHealthy(getObjectId())) {
			setInvalid();
			return;
		}

		outputTemperature = calculateOutputTemperature(*inputValue);
		outputTemperature.setValid(PoolVariableIF::VALID);

		timeval uptime;
		Clock::getUptime(&uptime);

		if (uptimeOfOldTemperature.tv_sec != INVALID_UPTIME) {
			//In theory, we could use an AbsValueMonitor to monitor the gradient.
			//But this would require storing the maxGradient in DP and quite some overhead.
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
		sensorMonitor->doCheck(outputTemperature.value);

		if (sensorMonitor->isOutOfLimits()) {
			uptimeOfOldTemperature.tv_sec = INVALID_UPTIME;
			outputTemperature.setValid(PoolVariableIF::INVALID);
			outputTemperature = INVALID_TEMPERATURE;
		} else {
			oldTemperature = outputTemperature;
			uptimeOfOldTemperature = uptime;
		}
	}

public:
	float getTemperature() {
		return outputTemperature;
	}

	bool isValid() {
		return outputTemperature.isValid();
	}

	static const uint16_t ADDRESS_A = 0;
	static const uint16_t ADDRESS_B = 1;
	static const uint16_t ADDRESS_C = 2;
	static const uint16_t ADDRESS_GRADIENT = 3;

	static const uint16_t DEFAULT_CONFIRMATION_COUNT = 1; //!< Changed due to issue with later temperature checking even tough the sensor monitor was confirming already (Was 10 before with comment = Correlates to a 10s confirmation time. Chosen rather large, should not be so bad for components and helps survive glitches.)

	static const uint8_t DOMAIN_ID_SENSOR = 1;

	virtual ReturnValue_t getParameter(uint8_t domainId, uint16_t parameterId,
			ParameterWrapper *parameterWrapper,
			const ParameterWrapper *newValues, uint16_t startAtIndex) {
		ReturnValue_t result = sensorMonitor->getParameter(domainId, parameterId,
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
		sensorMonitor->setToUnchecked();
	}

};

#endif /* TEMPERATURESENSOR_H_ */

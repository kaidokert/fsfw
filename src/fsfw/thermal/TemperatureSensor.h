#ifndef FSFW_THERMAL_TEMPERATURESENSOR_H_
#define FSFW_THERMAL_TEMPERATURESENSOR_H_

#include "../datapoollocal/LocalPoolDataSetBase.h"
#include "../datapoollocal/LocalPoolVariable.h"
#include "../monitoring/LimitMonitor.h"
#include "AbstractTemperatureSensor.h"
#include "tcsDefinitions.h"

/**
 * @brief 	This building block handles non-linear value conversion and
 * 			range checks for analog temperature sensors.
 * @details This class can be used to perform all necessary tasks for temperature sensors.
 *          A sensor can be instantiated by calling the constructor.
 *          The temperature is calculated from an input value with
 *			the calculateOutputTemperature() function. Range checking and
 *			limit monitoring is performed automatically.
 *			The inputType specifies the type of the raw input while the
 *			limitType specifies the type of the upper and lower limit to check against.
 * @ingroup thermal
 */

template <typename inputType, typename limitType = inputType>
class TemperatureSensor : public AbstractTemperatureSensor {
 public:
  /**
   * This structure contains parameters required for range checking
   * and the conversion from the input value to the output temperature.
   * a, b and c can be any parameters required to calculate the output
   * temperature from the input value, depending on the formula used.
   *
   * The parameters a,b and c are used in the calculateOutputTemperature() call.
   *
   * The lower and upper limits can be specified in any type, for example float for C values
   * or any other type for raw values.
   */
  struct Parameters {
    float a;
    float b;
    float c;
    limitType lowerLimit;
    limitType upperLimit;
    float maxGradient;
  };

  /**
   * Forward declaration for explicit instantiation of used parameters.
   */
  struct UsedParameters {
    UsedParameters(Parameters parameters)
        : a(parameters.a), b(parameters.b), c(parameters.c), gradient(parameters.maxGradient) {}
    float a;
    float b;
    float c;
    float gradient;
  };

  /**
   * Instantiate Temperature Sensor Object.
   * @param setObjectid       objectId of the sensor object
   * @param inputTemperature  Pointer to a raw input value which is converted to an floating
   *                          point C output temperature
   * @param outputGpid        Global Pool ID of the output value
   * @param vectorIndex       Vector Index for the sensor monitor
   * @param parameters        Calculation parameters, temperature limits, gradient limit
   * @param outputSet         Output dataset for the output temperature to fetch it with read()
   * @param thermalModule     Respective thermal module, if it has one
   */
  TemperatureSensor(object_id_t setObjectid, lp_var_t<limitType> *inputTemperature,
                    gp_id_t outputGpid, uint8_t vectorIndex,
                    Parameters parameters = {0, 0, 0, 0, 0, 0},
                    LocalPoolDataSetBase *outputSet = nullptr,
                    ThermalModuleIF *thermalModule = nullptr)
      : AbstractTemperatureSensor(setObjectid, thermalModule),
        parameters(parameters),
        inputTemperature(inputTemperature),
        outputTemperature(outputGpid, outputSet, PoolVariableIF::VAR_WRITE),
        sensorMonitor(setObjectid, DOMAIN_ID_SENSOR, outputGpid, DEFAULT_CONFIRMATION_COUNT,
                      parameters.lowerLimit, parameters.upperLimit, TEMP_SENSOR_LOW,
                      TEMP_SENSOR_HIGH),
        oldTemperature(20),
        uptimeOfOldTemperature({thermal::INVALID_TEMPERATURE, 0}) {}

 protected:
  /**
   * This formula is used to calculate the temperature from an input value
   * with an arbitrary type.
   * A default implementation is provided but can be replaced depending
   * on the required calculation.
   * @param inputTemperature
   * @return
   */
  virtual float calculateOutputTemperature(inputType inputValue) {
    return parameters.a * inputValue * inputValue + parameters.b * inputValue + parameters.c;
  }

 private:
  void setInvalid() {
    outputTemperature = thermal::INVALID_TEMPERATURE;
    outputTemperature.setValid(false);
    uptimeOfOldTemperature.tv_sec = INVALID_UPTIME;
    sensorMonitor.setToInvalid();
  }

 protected:
  static const int32_t INVALID_UPTIME = 0;

  UsedParameters parameters;

  lp_var_t<limitType> *inputTemperature;
  lp_var_t<float> outputTemperature;

  LimitMonitor<limitType> sensorMonitor;

  float oldTemperature;
  timeval uptimeOfOldTemperature;

  void doChildOperation() {
    ReturnValue_t result = inputTemperature->read(MutexIF::TimeoutType::WAITING, 20);
    if (result != returnvalue::OK) {
      return;
    }

    if ((not inputTemperature->isValid()) or
        (not healthHelper.healthTable->isHealthy(getObjectId()))) {
      setInvalid();
      return;
    }

    outputTemperature = calculateOutputTemperature(inputTemperature->value);
    outputTemperature.setValid(PoolVariableIF::VALID);

    timeval uptime;
    Clock::getUptime(&uptime);

    if (uptimeOfOldTemperature.tv_sec != INVALID_UPTIME) {
      // In theory, we could use an AbsValueMonitor to monitor the gradient.
      // But this would require storing the maxGradient in DP and quite some overhead.
      // The concept of delta limits is a bit strange anyway.
      float deltaTime;
      float deltaTemp;

      deltaTime = (uptime.tv_sec + uptime.tv_usec / 1000000.) -
                  (uptimeOfOldTemperature.tv_sec + uptimeOfOldTemperature.tv_usec / 1000000.);
      deltaTemp = oldTemperature - outputTemperature;
      if (deltaTemp < 0) {
        deltaTemp = -deltaTemp;
      }
      if (parameters.gradient < deltaTemp / deltaTime) {
        triggerEvent(TEMP_SENSOR_GRADIENT);
        // Don't set invalid, as we did not recognize it as invalid with full authority,
        // let FDIR handle it
      }
    }

    sensorMonitor.doCheck(outputTemperature.value);

    if (sensorMonitor.isOutOfLimits()) {
      uptimeOfOldTemperature.tv_sec = INVALID_UPTIME;
      outputTemperature.setValid(PoolVariableIF::INVALID);
      outputTemperature = thermal::INVALID_TEMPERATURE;
    } else {
      oldTemperature = outputTemperature;
      uptimeOfOldTemperature = uptime;
    }
  }

 public:
  float getTemperature() { return outputTemperature; }

  bool isValid() { return outputTemperature.isValid(); }

  static const uint16_t ADDRESS_A = 0;
  static const uint16_t ADDRESS_B = 1;
  static const uint16_t ADDRESS_C = 2;
  static const uint16_t ADDRESS_GRADIENT = 3;

  //! Changed due to issue with later temperature checking even tough the sensor monitor was
  //! confirming already (Was 10 before with comment = Correlates to a 10s confirmation time.
  //! Chosen rather large, should not be so bad for components and helps survive glitches.)
  static const uint16_t DEFAULT_CONFIRMATION_COUNT = 1;

  static const uint8_t DOMAIN_ID_SENSOR = 1;

  virtual ReturnValue_t getParameter(uint8_t domainId, uint8_t uniqueId,
                                     ParameterWrapper *parameterWrapper,
                                     const ParameterWrapper *newValues, uint16_t startAtIndex) {
    ReturnValue_t result =
        sensorMonitor.getParameter(domainId, uniqueId, parameterWrapper, newValues, startAtIndex);
    if (result != INVALID_DOMAIN_ID) {
      return result;
    }
    if (domainId != this->DOMAIN_ID_BASE) {
      return INVALID_DOMAIN_ID;
    }
    switch (uniqueId) {
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
        return INVALID_IDENTIFIER_ID;
    }
    return returnvalue::OK;
  }

  virtual void resetOldState() { sensorMonitor.setToUnchecked(); }
};

#endif /* FSFW_THERMAL_TEMPERATURESENSOR_H_ */

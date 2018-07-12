#ifndef THERMALCOMPONENT_H_
#define THERMALCOMPONENT_H_

#include "CoreComponent.h"

class ThermalComponent: public CoreComponent {
public:
	struct Parameters {
		float lowerNopLimit;
		float lowerOpLimit;
		float upperOpLimit;
		float upperNopLimit;
		float heaterOn;
		float hysteresis;
		float heaterSwitchoff;
	};
	struct NopParameters {
		float lowerNopLimit;
		float upperNopLimit;
	};
	ThermalComponent(object_id_t reportingObjectId, uint8_t domainId, uint32_t temperaturePoolId,
			uint32_t targetStatePoolId, uint32_t currentStatePoolId, uint32_t requestPoolId,
			DataSet *dataSet, AbstractTemperatureSensor *sensor,
			AbstractTemperatureSensor *firstRedundantSensor,
			AbstractTemperatureSensor *secondRedundantSensor,
			ThermalModuleIF *thermalModule, Parameters parameters,
			Priority priority);
	virtual ~ThermalComponent();

	ReturnValue_t setTargetState(int8_t newState);

	virtual ReturnValue_t setLimits( const uint8_t* data, uint32_t size);

	virtual ReturnValue_t getParameter(uint8_t domainId, uint16_t parameterId,
				ParameterWrapper *parameterWrapper,
				const ParameterWrapper *newValues, uint16_t startAtIndex);

protected:

	NopParameters nopParameters;

	State getState(float temperature, CoreComponent::Parameters parameters,
			int8_t targetState);

	virtual void checkLimits(State state);

	virtual HeaterRequest getHeaterRequest(int8_t targetState, float temperature,
			CoreComponent::Parameters parameters);

	State getIgnoredState(int8_t state);
};

#endif /* THERMALCOMPONENT_H_ */

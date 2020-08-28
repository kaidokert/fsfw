#ifndef THERMALMODULE_H_
#define THERMALMODULE_H_

#include "../datapoolglob/GlobalDataSet.h"
#include "../datapoolglob/GlobalPoolVariable.h"
#include "../devicehandlers/HealthDevice.h"
#include "../events/EventReportingProxyIF.h"
#include "ThermalModuleIF.h"
#include <list>
#include "tcsDefinitions.h"
#include "RedundantHeater.h"
class PowerSwitchIF;

/**
 * @brief Allows creation of different thermal control domains within a system.
 */
class ThermalModule: public ThermalModuleIF {
	friend class ThermalController;
public:
	struct Parameters {
		float heaterOn;
		float hysteresis;
	};

	ThermalModule(uint32_t moduleTemperaturePoolId, uint32_t currentStatePoolId,
			uint32_t targetStatePoolId, GlobDataSet *dataSet, Parameters parameters,
			RedundantHeater::Parameters heaterParameters);

	ThermalModule(uint32_t moduleTemperaturePoolId, GlobDataSet *dataSet);

	virtual ~ThermalModule();

	void performOperation(uint8_t opCode);

	void performMode(Strategy strategy);

	float getTemperature();

	void registerSensor(AbstractTemperatureSensor *sensor);

	void registerComponent(ThermalComponentIF *component,
			ThermalComponentIF::Priority priority);

	ThermalComponentIF *findComponent(object_id_t objectId);

	void initialize(PowerSwitchIF* powerSwitch);

	void setHeating(bool on);

	virtual void setOutputInvalid();

protected:
	enum Informee {
		ALL, SAFE, NONE
	};

	struct ComponentData {
		ThermalComponentIF *component;
		ThermalComponentIF::Priority priority;
		ThermalComponentIF::HeaterRequest request;
	};

	Strategy oldStrategy;

	float survivalTargetTemp;

	float targetTemp;

	bool heating;

	Parameters parameters;

	gp_float_t moduleTemperature;

	RedundantHeater *heater;

	gp_int8_t currentState;
	gp_int8_t targetState;

	std::list<AbstractTemperatureSensor *> sensors;
	std::list<ComponentData> components;

	void calculateTemperature();

	ThermalComponentIF::HeaterRequest letComponentsPerformAndDeciceIfWeNeedToHeat(bool safeOnly);

	void informComponentsAboutHeaterState(bool heaterIsOn,
			Informee whomToInform);

	bool calculateModuleHeaterRequestAndSetModuleStatus(Strategy strategy);

	void updateTargetTemperatures(ThermalComponentIF *component, bool isSafe);
};

#endif /* THERMALMODULE_H_ */

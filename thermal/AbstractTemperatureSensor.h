#ifndef ABSTRACTSENSOR_H_
#define ABSTRACTSENSOR_H_

#include "../health/HasHealthIF.h"
#include "../health/HealthHelper.h"
#include "../objectmanager/SystemObject.h"
#include "../tasks/ExecutableObjectIF.h"
#include "../parameters/ParameterHelper.h"
#include "../ipc/MessageQueueIF.h"
#include "ThermalModuleIF.h"
#include "tcsDefinitions.h"

class AbstractTemperatureSensor: public HasHealthIF,
		public SystemObject,
		public ExecutableObjectIF,
		public ReceivesParameterMessagesIF {
public:

	static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::T_SENSORS;
	static const Event TEMP_SENSOR_HIGH = MAKE_EVENT(0, SEVERITY::LOW);
	static const Event TEMP_SENSOR_LOW = MAKE_EVENT(1, SEVERITY::LOW);
	static const Event TEMP_SENSOR_GRADIENT = MAKE_EVENT(2, SEVERITY::LOW);

	static constexpr float ZERO_KELVIN_C = -273.15;
	AbstractTemperatureSensor(object_id_t setObjectid,
			ThermalModuleIF *thermalModule);
	virtual ~AbstractTemperatureSensor();

	virtual MessageQueueId_t getCommandQueue() const;

	ReturnValue_t initialize();

	ReturnValue_t performHealthOp();

	ReturnValue_t performOperation(uint8_t opCode);

	virtual float getTemperature() = 0;
	virtual bool isValid() = 0;

	virtual void resetOldState() = 0;

	ReturnValue_t setHealth(HealthState health);
	HasHealthIF::HealthState getHealth();
protected:
	MessageQueueIF* commandQueue;
	HealthHelper healthHelper;
	ParameterHelper parameterHelper;

	virtual void doChildOperation() = 0;

	void handleCommandQueue();
};

#endif /* ABSTRACTSENSOR_H_ */

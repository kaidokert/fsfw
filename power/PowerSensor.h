#ifndef POWERSENSOR_H_
#define POWERSENSOR_H_

#include "../datapool/DataSet.h"
#include "../datapool/PIDReader.h"
#include "../datapool/PoolVariable.h"
#include "../devicehandlers/HealthDevice.h"
#include "../monitoring/LimitMonitor.h"
#include "../parameters/ParameterHelper.h"
#include "../objectmanager/SystemObject.h"
#include "../ipc/MessageQueueIF.h"

class PowerController;

class PowerSensor: public SystemObject,
		public ReceivesParameterMessagesIF,
		public HasHealthIF {
	friend class PowerController;
public:
	struct VariableIds {
		uint32_t pidCurrent;
		uint32_t pidVoltage;
		uint32_t poolIdPower;
	};
	struct DefaultLimits {
		float currentMin;
		float currentMax;
		float voltageMin;
		float voltageMax;
	};
	struct SensorEvents {
		Event currentLow;
		Event currentHigh;
		Event voltageLow;
		Event voltageHigh;
	};
	PowerSensor(object_id_t setId, VariableIds setIds, DefaultLimits limits,
			SensorEvents events, uint16_t confirmationCount = 0);
	virtual ~PowerSensor();
	ReturnValue_t calculatePower();
	ReturnValue_t performOperation(uint8_t opCode);
	void setAllMonitorsToUnchecked();
	MessageQueueId_t getCommandQueue() const;
	ReturnValue_t initialize();
	void setDataPoolEntriesInvalid();
	float getPower();
	ReturnValue_t setHealth(HealthState health);
	HasHealthIF::HealthState getHealth();
	ReturnValue_t getParameter(uint8_t domainId, uint16_t parameterId,
			ParameterWrapper *parameterWrapper,
			const ParameterWrapper *newValues, uint16_t startAtIndex);
private:
	MessageQueueIF* commandQueue;
	ParameterHelper parameterHelper;
	HealthHelper healthHelper;
	DataSet set;
	//Variables in
	PIDReader<float> current;
	PIDReader<float> voltage;
	//Variables out
	db_float_t power;

	static const uint8_t MODULE_ID_CURRENT = 1;
	static const uint8_t MODULE_ID_VOLTAGE = 2;
	void checkCommandQueue();
protected:
	LimitMonitor<float> currentLimit;
	LimitMonitor<float> voltageLimit;
};

#endif /* POWERSENSOR_H_ */

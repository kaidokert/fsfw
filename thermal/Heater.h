#ifndef FRAMEWORK_THERMAL_HEATER_H_
#define FRAMEWORK_THERMAL_HEATER_H_

#include "../devicehandlers/HealthDevice.h"
#include "../parameters/ParameterHelper.h"
#include "../power/PowerSwitchIF.h"
#include "../returnvalues/HasReturnvaluesIF.h"
#include "../timemanager/Countdown.h"
#include <stdint.h>
//class RedundantHeater;

class Heater: public HealthDevice, public ReceivesParameterMessagesIF {
	friend class RedundantHeater;
public:

	static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::HEATER;
	static const Event HEATER_ON = MAKE_EVENT(0, SEVERITY::INFO);
	static const Event HEATER_OFF = MAKE_EVENT(1, SEVERITY::INFO);
	static const Event HEATER_TIMEOUT = MAKE_EVENT(2, SEVERITY::LOW);
	static const Event HEATER_STAYED_ON = MAKE_EVENT(3, SEVERITY::LOW);
	static const Event HEATER_STAYED_OFF = MAKE_EVENT(4, SEVERITY::LOW);

	Heater(uint32_t objectId, uint8_t switch0, uint8_t switch1);
	virtual ~Heater();

	ReturnValue_t performOperation(uint8_t opCode);

	ReturnValue_t initialize();

	ReturnValue_t set();
	void clear(bool passive);

	void setPowerSwitcher(PowerSwitchIF *powerSwitch);

	MessageQueueId_t getCommandQueue() const;

	ReturnValue_t getParameter(uint8_t domainId, uint16_t parameterId,
			ParameterWrapper *parameterWrapper,
			const ParameterWrapper *newValues, uint16_t startAtIndex);

protected:
	static const uint32_t INVALID_UPTIME = 0;

	enum InternalState {
		STATE_ON,
		STATE_OFF,
		STATE_PASSIVE,
		STATE_WAIT_FOR_SWITCHES_ON,
		STATE_WAIT_FOR_SWITCHES_OFF,
		STATE_WAIT_FOR_FDIR, //used to avoid doing anything until fdir decided what to do
		STATE_FAULTY,
		STATE_WAIT, //used when waiting for system to recover from miniops
		STATE_EXTERNAL_CONTROL //entered when under external control and a fdir reaction would be triggered. This is useful when leaving external control into an unknown state
							   //if no fdir reaction is triggered under external control the state is still ok and no need for any special treatment is needed
	} internalState;

	PowerSwitchIF *powerSwitcher;
	MessageQueueId_t pcduQueueId;

	uint8_t switch0;
	uint8_t switch1;

	bool wasOn;

	bool timedOut;

	bool reactedToBeingFaulty;

	bool passive;

	MessageQueueIF* eventQueue;
	Countdown heaterOnCountdown;
	Countdown switchCountdown;
	ParameterHelper parameterHelper;

	enum Action {
		SET, CLEAR
	} lastAction;

	void doAction(Action action);

	void setSwitch(uint8_t number, ReturnValue_t state,
			uint32_t *upTimeOfSwitching);

	void handleQueue();

	void handleEventQueue();
};

#endif /* FRAMEWORK_THERMAL_HEATER_H_ */

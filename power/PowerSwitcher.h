#ifndef POWERSWITCHER_H_
#define POWERSWITCHER_H_
#include "PowerSwitchIF.h"
#include "../returnvalues/HasReturnvaluesIF.h"
#include "../timemanager/Countdown.h"

class PowerSwitcher : public HasReturnvaluesIF {
public:
	enum State_t {
		WAIT_OFF,
		WAIT_ON,
		SWITCH_IS_OFF,
		SWITCH_IS_ON,
	};
	State_t state;
	static const uint8_t INTERFACE_ID = CLASS_ID::POWER_SWITCHER;
	static const ReturnValue_t IN_POWER_TRANSITION = MAKE_RETURN_CODE(1);
	static const ReturnValue_t SWITCH_STATE_MISMATCH = MAKE_RETURN_CODE(2);
	PowerSwitcher( uint8_t setSwitch1, uint8_t setSwitch2 = NO_SWITCH, State_t setStartState = SWITCH_IS_OFF );
	ReturnValue_t initialize(object_id_t powerSwitchId);
	void turnOn();
	void turnOff();
	void doStateMachine();
	State_t getState();
	ReturnValue_t checkSwitchState();
	uint32_t getSwitchDelay();
	uint8_t getFirstSwitch() const;
	uint8_t getSecondSwitch() const;
private:
	uint8_t firstSwitch;
	uint8_t secondSwitch;
	PowerSwitchIF* power;
	static const uint8_t NO_SWITCH = 0xFF;
	enum SwitchReturn_t {
		ONE_SWITCH = 1,
		TWO_SWITCHES = 2
	};
	ReturnValue_t getStateOfSwitches();
	void commandSwitches( ReturnValue_t onOff );
	SwitchReturn_t howManySwitches();
};



#endif /* POWERSWITCHER_H_ */

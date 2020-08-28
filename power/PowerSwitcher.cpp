#include "../objectmanager/ObjectManagerIF.h"
#include "PowerSwitcher.h"
#include "../serviceinterface/ServiceInterfaceStream.h"

PowerSwitcher::PowerSwitcher(uint8_t setSwitch1, uint8_t setSwitch2,
		PowerSwitcher::State_t setStartState) :
		state(setStartState), firstSwitch(setSwitch1), secondSwitch(setSwitch2), power(NULL) {
}

ReturnValue_t PowerSwitcher::initialize(object_id_t powerSwitchId) {
	power = objectManager->get<PowerSwitchIF>(powerSwitchId);
	if (power == NULL) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t PowerSwitcher::getStateOfSwitches() {
	SwitchReturn_t result = howManySwitches();
	switch (result) {
	case ONE_SWITCH:
		return power->getSwitchState(firstSwitch);
	case TWO_SWITCHES:
		if ((power->getSwitchState(firstSwitch) == PowerSwitchIF::SWITCH_ON)
				&& (power->getSwitchState(secondSwitch) == PowerSwitchIF::SWITCH_ON)) {
			return PowerSwitchIF::SWITCH_ON;
		} else if ((power->getSwitchState(firstSwitch) == PowerSwitchIF::SWITCH_OFF)
				&& (power->getSwitchState(secondSwitch) == PowerSwitchIF::SWITCH_OFF)) {
			return PowerSwitchIF::SWITCH_OFF;
		} else {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
	default:
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

void PowerSwitcher::commandSwitches(ReturnValue_t onOff) {
	SwitchReturn_t result = howManySwitches();
	switch (result) {
	case TWO_SWITCHES:
		power->sendSwitchCommand(secondSwitch, onOff);
		/* NO BREAK falls through*/
	case ONE_SWITCH:
		power->sendSwitchCommand(firstSwitch, onOff);
		break;
	}
	return;

}

void PowerSwitcher::turnOn() {
	commandSwitches(PowerSwitchIF::SWITCH_ON);
	state = WAIT_ON;
}

void PowerSwitcher::turnOff() {
	commandSwitches(PowerSwitchIF::SWITCH_OFF);
	state = WAIT_OFF;
}

PowerSwitcher::SwitchReturn_t PowerSwitcher::howManySwitches() {
	if (secondSwitch == NO_SWITCH) {
		return ONE_SWITCH;
	} else {
		return TWO_SWITCHES;
	}
}

void PowerSwitcher::doStateMachine() {
	switch (state) {
	case SWITCH_IS_OFF:
	case SWITCH_IS_ON:
		//Do nothing.
		break;
	case WAIT_OFF:
		if (getStateOfSwitches() == PowerSwitchIF::SWITCH_OFF) {
			state = SWITCH_IS_OFF;
		}
		break;
	case WAIT_ON:
		if (getStateOfSwitches() == PowerSwitchIF::SWITCH_ON) {
			state = SWITCH_IS_ON;
		}
		break;
	default:
		//Should never happen.
		break;
	}
}

ReturnValue_t PowerSwitcher::checkSwitchState() {
	switch (state) {
	case WAIT_OFF:
	case WAIT_ON:
		return IN_POWER_TRANSITION;
	case SWITCH_IS_OFF:
		if (getStateOfSwitches() == PowerSwitchIF::SWITCH_OFF) {
			return RETURN_OK;
		} else {
			return SWITCH_STATE_MISMATCH;
		}
	case SWITCH_IS_ON:
		if (getStateOfSwitches() == PowerSwitchIF::SWITCH_ON) {
			return RETURN_OK;
		} else {
			return SWITCH_STATE_MISMATCH;
		}
	}
	return RETURN_FAILED;
}

PowerSwitcher::State_t PowerSwitcher::getState() {
	return state;
}

uint32_t PowerSwitcher::getSwitchDelay() {
	return power->getSwitchDelayMs();
}

uint8_t PowerSwitcher::getFirstSwitch() const {
	return firstSwitch;
}

uint8_t PowerSwitcher::getSecondSwitch() const {
	return secondSwitch;
}

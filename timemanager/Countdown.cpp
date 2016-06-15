/**
 * @file	Countdown.cpp
 * @brief	This file defines the Countdown class.
 * @date	21.03.2013
 * @author	baetz
 */


#include <framework/timemanager/Countdown.h>

Countdown::Countdown(uint32_t initialTimeout) : startTime(0), timeout(initialTimeout) {
}

Countdown::~Countdown() {
}

ReturnValue_t Countdown::setTimeout(uint32_t miliseconds) {
	ReturnValue_t return_value = OSAL::getUptime( &startTime );
	timeout = miliseconds;
	return return_value;
}

bool Countdown::hasTimedOut() const {
	uint32_t current_time;
	OSAL::getUptime( &current_time );
	if ( uint32_t(current_time - startTime) >= timeout) {
		return true;
	} else {
		return false;
	}
}

bool Countdown::isBusy() const {
	return !hasTimedOut();
}

ReturnValue_t Countdown::resetTimer() {
	return setTimeout(timeout);
}

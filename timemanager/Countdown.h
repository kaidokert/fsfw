#ifndef FSFW_TIMEMANAGER_COUNTDOWN_H_
#define FSFW_TIMEMANAGER_COUNTDOWN_H_

#include "Clock.h"

/**
 * @brief	This file defines the Countdown class.
 * @author	baetz
 */
class Countdown {
public:
	uint32_t timeout;
	Countdown(uint32_t initialTimeout = 0);
	~Countdown();
	ReturnValue_t setTimeout(uint32_t miliseconds);

	bool hasTimedOut() const;

	bool isBusy() const;

	//!< Use last set timeout value and restart timer.
	ReturnValue_t resetTimer();

	//!< Make hasTimedOut() return true
	void timeOut();

private:
	uint32_t startTime = 0;
};

#endif /* FSFW_TIMEMANAGER_COUNTDOWN_H_ */

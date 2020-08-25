/**
 * @file	Countdown.h
 * @brief	This file defines the Countdown class.
 * @date	21.03.2013
 * @author	baetz
 */

#ifndef COUNTDOWN_H_
#define COUNTDOWN_H_

#include "Clock.h"

class Countdown {
private:
	uint32_t startTime;
public:
	uint32_t timeout;
	Countdown(uint32_t initialTimeout = 0);
	~Countdown();
	ReturnValue_t setTimeout(uint32_t miliseconds);

	bool hasTimedOut() const;

	bool isBusy() const;

	ReturnValue_t resetTimer(); //!< Use last set timeout value and restart timer.

	void timeOut(); //!< Make hasTimedOut() return true
};

#endif /* COUNTDOWN_H_ */

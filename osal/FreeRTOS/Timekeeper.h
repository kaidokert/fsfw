#ifndef FRAMEWORK_OSAL_FREERTOS_TIMEKEEPER_H_
#define FRAMEWORK_OSAL_FREERTOS_TIMEKEEPER_H_

#include <framework/timemanager/Clock.h>

#include <FreeRTOS.h>

/**
 * A Class to basically store the time difference between uptime and UTC
 * so the "time-agnostic" FreeRTOS can keep an UTC Time
 *
 * Implemented as Singleton, so the FSFW Clock Implementation (see Clock.cpp)
 * can use it without having a member.
 */

class Timekeeper {
private:
	Timekeeper();

	timeval offset;

	static Timekeeper * myinstance;
public:
	static Timekeeper * instance();
	virtual ~Timekeeper();

	static timeval ticksToTimeval(TickType_t ticks);

	const timeval& getOffset() const;
	void setOffset(const timeval& offset);
};

#endif /* FRAMEWORK_OSAL_FREERTOS_TIMEKEEPER_H_ */

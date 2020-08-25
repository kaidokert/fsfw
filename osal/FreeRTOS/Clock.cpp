#include "../../timemanager/Clock.h"
#include "../../globalfunctions/timevalOperations.h"
#include "Timekeeper.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <stdlib.h>
#include <time.h>

//TODO sanitize input?
//TODO much of this code can be reused for tick-only systems

uint16_t Clock::leapSeconds = 0;
MutexIF* Clock::timeMutex = nullptr;

uint32_t Clock::getTicksPerSecond(void) {
	return 1000;
}

ReturnValue_t Clock::setClock(const TimeOfDay_t* time) {

	timeval time_timeval;

	ReturnValue_t result = convertTimeOfDayToTimeval(time, &time_timeval);
	if (result != HasReturnvaluesIF::RETURN_OK){
		return result;
	}

	return setClock(&time_timeval);
}

ReturnValue_t Clock::setClock(const timeval* time) {
	timeval uptime = getUptime();

	timeval offset = *time - uptime;

	Timekeeper::instance()->setOffset(offset);

	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::getClock_timeval(timeval* time) {
	timeval uptime = getUptime();

	timeval offset = Timekeeper::instance()->getOffset();

	*time = offset + uptime;

	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::getUptime(timeval* uptime) {
	*uptime = getUptime();

	return HasReturnvaluesIF::RETURN_OK;
}

timeval Clock::getUptime() {
	TickType_t ticksSinceStart = xTaskGetTickCount();
	return Timekeeper::ticksToTimeval(ticksSinceStart);
}

ReturnValue_t Clock::getUptime(uint32_t* uptimeMs) {
	timeval uptime = getUptime();
	*uptimeMs = uptime.tv_sec * 1000 + uptime.tv_usec / 1000;
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::getClock_usecs(uint64_t* time) {
	timeval time_timeval;
	ReturnValue_t result = getClock_timeval(&time_timeval);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	*time = time_timeval.tv_sec * 1000000 + time_timeval.tv_usec;
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::getDateAndTime(TimeOfDay_t* time) {
	timeval time_timeval;
	ReturnValue_t result = getClock_timeval(&time_timeval);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	struct tm time_tm;

	gmtime_r(&time_timeval.tv_sec,&time_tm);

	time->year = time_tm.tm_year + 1900;
	time->month = time_tm.tm_mon + 1;
	time->day = time_tm.tm_mday;

	time->hour = time_tm.tm_hour;
	time->minute = time_tm.tm_min;
	time->second = time_tm.tm_sec;

	time->usecond = time_timeval.tv_usec;


	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::convertTimeOfDayToTimeval(const TimeOfDay_t* from,
		timeval* to) {
	struct tm time_tm;

		time_tm.tm_year = from->year - 1900;
		time_tm.tm_mon = from->month - 1;
		time_tm.tm_mday = from->day;

		time_tm.tm_hour = from->hour;
		time_tm.tm_min = from->minute;
		time_tm.tm_sec = from->second;

		time_t seconds = mktime(&time_tm);

		to->tv_sec = seconds;
		to->tv_usec = from->usecond;
	//Fails in 2038..
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::convertTimevalToJD2000(timeval time, double* JD2000) {
	*JD2000 = (time.tv_sec - 946728000. + time.tv_usec / 1000000.) / 24.
			/ 3600.;
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::convertUTCToTT(timeval utc, timeval* tt) {
	//SHOULDDO: works not for dates in the past (might have less leap seconds)
	if (timeMutex == nullptr) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	uint16_t leapSeconds;
	ReturnValue_t result = getLeapSeconds(&leapSeconds);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	timeval leapSeconds_timeval = { 0, 0 };
	leapSeconds_timeval.tv_sec = leapSeconds;

	//initial offset between UTC and TAI
	timeval UTCtoTAI1972 = { 10, 0 };

	timeval TAItoTT = { 32, 184000 };

	*tt = utc + leapSeconds_timeval + UTCtoTAI1972 + TAItoTT;

	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::setLeapSeconds(const uint16_t leapSeconds_) {
	if (checkOrCreateClockMutex() != HasReturnvaluesIF::RETURN_OK) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	ReturnValue_t result = timeMutex->lockMutex(MutexIF::BLOCKING);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	leapSeconds = leapSeconds_;

	result = timeMutex->unlockMutex();
	return result;
}

ReturnValue_t Clock::getLeapSeconds(uint16_t* leapSeconds_) {
	if (timeMutex == NULL) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	ReturnValue_t result = timeMutex->lockMutex(MutexIF::BLOCKING);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	*leapSeconds_ = leapSeconds;

	result = timeMutex->unlockMutex();
	return result;
}

ReturnValue_t Clock::checkOrCreateClockMutex() {
	if (timeMutex == NULL) {
		MutexFactory* mutexFactory = MutexFactory::instance();
		if (mutexFactory == NULL) {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
		timeMutex = mutexFactory->createMutex();
		if (timeMutex == NULL) {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
	}
	return HasReturnvaluesIF::RETURN_OK;
}

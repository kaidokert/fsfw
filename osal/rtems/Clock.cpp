#include <framework/timemanager/Clock.h>
#include "RtemsBasic.h"

uint16_t Clock::leapSeconds = 0;
MutexIF* Clock::timeMutex = NULL;

uint32_t Clock::getTicksPerSecond(void){
	rtems_interval ticks_per_second;
	(void) rtems_clock_get(RTEMS_CLOCK_GET_TICKS_PER_SECOND, &ticks_per_second);
	return ticks_per_second;
}

ReturnValue_t Clock::setClock(const TimeOfDay_t* time) {
	//We need to cast to rtems internal time of day type here. Both structs have the same structure
	//rtems provides no const guarantee, so we need to cast the const away
	//TODO Check if this can be done safely
	rtems_time_of_day* timeRtems = reinterpret_cast<rtems_time_of_day*>(const_cast<TimeOfDay_t*>(time));
	rtems_status_code status = rtems_clock_set(timeRtems);
	return RtemsBasic::convertReturnCode(status);
}

ReturnValue_t Clock::setClock(const timeval* time) {
	timespec newTime;
	newTime.tv_sec = time->tv_sec;
	newTime.tv_nsec = time->tv_usec * TOD_NANOSECONDS_PER_MICROSECOND;
	//SHOULDDO: Not sure if we need to protect this call somehow (by thread lock or something).
	//Uli: rtems docu says you can call this from an ISR, not sure if this means no protetion needed
	_TOD_Set(&newTime);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::getClock_timeval(timeval* time) {
	rtems_status_code status = rtems_clock_get_tod_timeval(time);
	return RtemsBasic::convertReturnCode(status);
}

ReturnValue_t Clock::getUptime(timeval* uptime) {
	timespec time;
	rtems_status_code status = rtems_clock_get_uptime(&time);
	uptime->tv_sec = time.tv_sec;
	time.tv_nsec = time.tv_nsec / 1000;
	uptime->tv_usec = time.tv_nsec;
	return RtemsBasic::convertReturnCode(status);
}

ReturnValue_t Clock::getUptime(uint32_t* uptimeMs) {
	*uptimeMs = rtems_clock_get_ticks_since_boot();
	return RtemsBasic::convertReturnCode(RTEMS_SUCCESSFUL);
}

ReturnValue_t Clock::getClock_usecs(uint64_t* time) {
	timeval temp_time;
	rtems_status_code returnValue = rtems_clock_get_tod_timeval(&temp_time);
	*time = ((uint64_t) temp_time.tv_sec * 1000000) + temp_time.tv_usec;
	return RtemsBasic::convertReturnCode(returnValue);
}

ReturnValue_t Clock::getDateAndTime(TimeOfDay_t* time) {
	rtems_time_of_day* timeRtems = reinterpret_cast<rtems_time_of_day*>(time);
	rtems_status_code status = rtems_clock_get_tod(timeRtems);
	return RtemsBasic::convertReturnCode(status);
}

ReturnValue_t Clock::convertTimeOfDayToTimeval(const TimeOfDay_t* from,
		timeval* to) {
	//Fails in 2038..
	const rtems_time_of_day* timeRtems = reinterpret_cast<const rtems_time_of_day*>(from);
	to->tv_sec = _TOD_To_seconds(timeRtems);
	to->tv_usec = timeRtems->ticks * 1000;
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::convertTimevalToJD2000(timeval time, double* JD2000) {
	*JD2000 = (time.tv_sec - 946728000. + time.tv_usec / 1000000.) / 24.
			/ 3600.;
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::convertUTCToTT(timeval utc, timeval* tt) {
	//SHOULDDO: works not for dates in the past (might have less leap seconds)
	if (timeMutex == NULL) {
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
	if(checkOrCreateClockMutex()!=HasReturnvaluesIF::RETURN_OK){
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	ReturnValue_t result = timeMutex->lockMutex(MutexIF::NO_TIMEOUT);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	leapSeconds = leapSeconds_;

	result = timeMutex->unlockMutex();
	return result;
}

ReturnValue_t Clock::getLeapSeconds(uint16_t* leapSeconds_) {
	if(timeMutex==NULL){
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	ReturnValue_t result = timeMutex->lockMutex(MutexIF::NO_TIMEOUT);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	*leapSeconds_ = leapSeconds;

	result = timeMutex->unlockMutex();
	return result;
}

ReturnValue_t Clock::checkOrCreateClockMutex(){
	if(timeMutex==NULL){
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

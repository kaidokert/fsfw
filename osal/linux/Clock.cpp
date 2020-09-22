#include "../../serviceinterface/ServiceInterfaceStream.h"
#include "../../timemanager/Clock.h"

#include <sys/time.h>
#include <sys/sysinfo.h>
#include <linux/sysinfo.h>
#include <time.h>
#include <unistd.h>
#include <fstream>

uint16_t Clock::leapSeconds = 0;
MutexIF* Clock::timeMutex = NULL;

uint32_t Clock::getTicksPerSecond(void){
	uint32_t ticks = sysconf(_SC_CLK_TCK);
	return ticks;
}

ReturnValue_t Clock::setClock(const TimeOfDay_t* time) {
	timespec timeUnix;
	timeval timeTimeval;
	convertTimeOfDayToTimeval(time,&timeTimeval);
	timeUnix.tv_sec = timeTimeval.tv_sec;
	timeUnix.tv_nsec = (__syscall_slong_t) timeTimeval.tv_usec * 1000;

	int status = clock_settime(CLOCK_REALTIME,&timeUnix);
	if(status!=0){
		//TODO errno
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::setClock(const timeval* time) {
	timespec timeUnix;
	timeUnix.tv_sec = time->tv_sec;
	timeUnix.tv_nsec = (__syscall_slong_t) time->tv_usec * 1000;
	int status = clock_settime(CLOCK_REALTIME,&timeUnix);
	if(status!=0){
		//TODO errno
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::getClock_timeval(timeval* time) {
	timespec timeUnix;
	int status = clock_gettime(CLOCK_REALTIME,&timeUnix);
	if(status!=0){
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	time->tv_sec = timeUnix.tv_sec;
	time->tv_usec = timeUnix.tv_nsec / 1000.0;
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::getClock_usecs(uint64_t* time) {
	timeval timeVal;
	ReturnValue_t result = getClock_timeval(&timeVal);
	if(result != HasReturnvaluesIF::RETURN_OK){
		return result;
	}
	*time = (uint64_t)timeVal.tv_sec*1e6 + timeVal.tv_usec;

	return HasReturnvaluesIF::RETURN_OK;
}

timeval Clock::getUptime() {
	timeval uptime;
	auto result = getUptime(&uptime);
	if(result != HasReturnvaluesIF::RETURN_OK) {
		sif::error << "Clock::getUptime: Error getting uptime" << std::endl;
	}
	return uptime;
}

ReturnValue_t Clock::getUptime(timeval* uptime) {
    //TODO This is not posix compatible and delivers only seconds precision
    // is the OS not called Linux?
    //Linux specific file read but more precise
    double uptimeSeconds;
    if(std::ifstream("/proc/uptime",std::ios::in) >> uptimeSeconds){
        uptime->tv_sec = uptimeSeconds;
        uptime->tv_usec = uptimeSeconds *(double) 1e6 - (uptime->tv_sec *1e6);
    }

	//TODO This is not posix compatible and delivers only seconds precision
    // I suggest this is moved into another clock function which will
    // deliver second precision later.
//	struct sysinfo sysInfo;
//	int result = sysinfo(&sysInfo);
//	if(result != 0){
//		return HasReturnvaluesIF::RETURN_FAILED;
//	}
//	return sysInfo.uptime;
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::getUptime(uint32_t* uptimeMs) {
	timeval uptime;
	ReturnValue_t result = getUptime(&uptime);
	if(result != HasReturnvaluesIF::RETURN_OK){
		return result;
	}
	*uptimeMs = uptime.tv_sec * 1e3 + uptime.tv_usec / 1e3;
	return HasReturnvaluesIF::RETURN_OK;
}



ReturnValue_t Clock::getDateAndTime(TimeOfDay_t* time) {
	timespec timeUnix;
	int status = clock_gettime(CLOCK_REALTIME,&timeUnix);
	if(status != 0){
		//TODO errno
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	struct tm* timeInfo;
	timeInfo = gmtime(&timeUnix.tv_sec);
	time->year = timeInfo->tm_year + 1900;
	time->month = timeInfo->tm_mon+1;
	time->day = timeInfo->tm_mday;
	time->hour = timeInfo->tm_hour;
	time->minute = timeInfo->tm_min;
	time->second = timeInfo->tm_sec;
	time->usecond = timeUnix.tv_nsec / 1000.0;

	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::convertTimeOfDayToTimeval(const TimeOfDay_t* from,
		timeval* to) {

	tm fromTm;
	//Note: Fails for years before AD
	fromTm.tm_year = from->year - 1900;
	fromTm.tm_mon = from->month - 1;
	fromTm.tm_mday = from->day;
	fromTm.tm_hour = from->hour;
	fromTm.tm_min = from->minute;
	fromTm.tm_sec = from->second;

	to->tv_sec = mktime(&fromTm);
	to->tv_usec = from->usecond;
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
	ReturnValue_t result = timeMutex->lockMutex(MutexIF::BLOCKING);
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
	ReturnValue_t result = timeMutex->lockMutex(MutexIF::BLOCKING);
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

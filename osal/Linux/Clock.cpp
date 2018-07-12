#include <sys/time.h>
#include <time.h>
#include <framework/timemanager/Clock.h>


#include <sys/sysinfo.h>
#include <linux/sysinfo.h>
#include <unistd.h>

//#include <fstream>


uint32_t Clock::getTicksPerSecond(void){
	//TODO This function returns the ticks per second for thread ticks not clock ticks
//	timespec ticks;
//	int status = clock_getres(CLOCK_REALTIME,&ticks);
//
//	if(status!=0){
//		//TODO errno
//		return 0xFFFFFFFF;
//	}
//	uint32_t resolution = 1e9 / ticks.tv_nsec;
	uint32_t resolution = sysconf(_SC_CLK_TCK);
	return resolution;
}

ReturnValue_t Clock::setClock(const TimeOfDay_t* time) {
	//TODO timeOfDay conversion
	timespec timeUnix;
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

ReturnValue_t Clock::getUptime(timeval* uptime) {
	//TODO This is not posix compatible and delivers only seconds precision
	struct sysinfo sysInfo;
	int result = sysinfo(&sysInfo);
	if(result != 0){
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	uptime->tv_sec = sysInfo.uptime;
	uptime->tv_usec = 0;


	//Linux specific file read but more precise
//	double uptimeSeconds;
//	if(std::ifstream("/proc/uptime",std::ios::in) >> uptimeSeconds){
//		uptime->tv_sec = uptimeSeconds;
//		uptime->tv_usec = uptimeSeconds *(double) 1e6 - (uptime->tv_sec *1e6);
//	}

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
	timespec ticks;
	status = clock_getres(CLOCK_REALTIME,&ticks);
	if(status!=0){
		//TODO errno
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	uint32_t resolution = 1e9 / ticks.tv_nsec;


	struct tm* timeInfo;
	timeInfo = gmtime(&timeUnix.tv_sec);
	time->year = timeInfo->tm_year + 1900;
	time->month = timeInfo->tm_mon+1;
	time->day = timeInfo->tm_mday;
	time->hour = timeInfo->tm_hour;
	time->minute = timeInfo->tm_min;
	time->second = timeInfo->tm_sec;
	time->ticks = (timeUnix.tv_nsec / (double) 1e9) * resolution;

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

	timespec ticks;
	uint32_t status = clock_getres(CLOCK_REALTIME,&ticks);
	if(status!=0){
		//TODO errno
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	uint32_t resolution = 1e9 / ticks.tv_nsec;

	to->tv_sec = mktime(&fromTm);
	to->tv_usec = (from->ticks /(double) resolution) * 1e6;



	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::convertTimevalToJD2000(timeval time, double* JD2000) {
	return HasReturnvaluesIF::RETURN_FAILED;
}

ReturnValue_t Clock::convertUTCToTT(timeval utc, timeval* tt) {
	return HasReturnvaluesIF::RETURN_FAILED;
}

ReturnValue_t Clock::setLeapSeconds(const uint16_t leapSeconds_) {
	return HasReturnvaluesIF::RETURN_FAILED;
}

ReturnValue_t Clock::getLeapSeconds(uint16_t* leapSeconds_) {
	return HasReturnvaluesIF::RETURN_FAILED;
}

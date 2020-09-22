#include "../../serviceinterface/ServiceInterfaceStream.h"
#include "../../timemanager/Clock.h"

#include <chrono>
#if defined(WIN32)
#include <windows.h>
#elif defined(LINUX)
#include <fstream>
#endif

uint16_t Clock::leapSeconds = 0;
MutexIF* Clock::timeMutex = NULL;

using SystemClock = std::chrono::system_clock;

uint32_t Clock::getTicksPerSecond(void){
	sif::warning << "Clock::getTicksPerSecond: not implemented yet" << std::endl;
	return 0;
	//return CLOCKS_PER_SEC;
	//uint32_t ticks = sysconf(_SC_CLK_TCK);
	//return ticks;
}

ReturnValue_t Clock::setClock(const TimeOfDay_t* time) {
	// do some magic with chrono
	sif::warning << "Clock::setClock: not implemented yet" << std::endl;
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::setClock(const timeval* time) {
	// do some magic with chrono
#if defined(WIN32)
	return HasReturnvaluesIF::RETURN_OK;
#elif defined(LINUX)
	return HasReturnvaluesIF::RETURN_OK;
#else

#endif
	sif::warning << "Clock::getUptime: Not implemented for found OS" << std::endl;
	return HasReturnvaluesIF::RETURN_FAILED;
}

ReturnValue_t Clock::getClock_timeval(timeval* time) {
#if defined(WIN32)
	auto now  = std::chrono::system_clock::now();
	auto secondsChrono = std::chrono::time_point_cast<std::chrono::seconds>(now);
	auto epoch = now.time_since_epoch();
	time->tv_sec = std::chrono::duration_cast<std::chrono::seconds>(epoch).count();
	auto fraction = now - secondsChrono;
	time->tv_usec = std::chrono::duration_cast<std::chrono::microseconds>(
	        fraction).count();
	return HasReturnvaluesIF::RETURN_OK;
#elif defined(LINUX)
	timespec timeUnix;
	int status = clock_gettime(CLOCK_REALTIME,&timeUnix);
	if(status!=0){
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	time->tv_sec = timeUnix.tv_sec;
	time->tv_usec = timeUnix.tv_nsec / 1000.0;
	return HasReturnvaluesIF::RETURN_OK;
#else
	sif::warning << "Clock::getUptime: Not implemented for found OS" << std::endl;
	return HasReturnvaluesIF::RETURN_FAILED;
#endif

}

ReturnValue_t Clock::getClock_usecs(uint64_t* time) {
	// do some magic with chrono
	sif::warning << "Clock::gerClock_usecs: not implemented yet" << std::endl;
	return HasReturnvaluesIF::RETURN_OK;
}

timeval Clock::getUptime() {
	timeval timeval;
#if defined(WIN32)
	auto uptime  = std::chrono::milliseconds(GetTickCount64());
	auto secondsChrono = std::chrono::duration_cast<std::chrono::seconds>(uptime);
	timeval.tv_sec = secondsChrono.count();
	auto fraction = uptime - secondsChrono;
	timeval.tv_usec = std::chrono::duration_cast<std::chrono::microseconds>(
	            fraction).count();
#elif defined(LINUX)
	double uptimeSeconds;
	if (std::ifstream("/proc/uptime", std::ios::in) >> uptimeSeconds)
	{
		// value is rounded down automatically
		timeval.tv_sec = uptimeSeconds;
		timeval.tv_usec = uptimeSeconds *(double) 1e6 - (timeval.tv_sec *1e6);
	}
#else
	sif::warning << "Clock::getUptime: Not implemented for found OS" << std::endl;
#endif
	return timeval;
}

ReturnValue_t Clock::getUptime(timeval* uptime) {
	*uptime = getUptime();
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::getUptime(uint32_t* uptimeMs) {
	timeval uptime = getUptime();
	*uptimeMs = uptime.tv_sec * 1000 + uptime.tv_usec / 1000;
	return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t Clock::getDateAndTime(TimeOfDay_t* time) {
	// do some magic with chrono (C++20!)
	// Right now, the library doesn't have the new features yet.
	// so we work around that for now.
	auto now = SystemClock::now();
	auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	auto fraction = now - seconds;
	time_t tt = SystemClock::to_time_t(now);
	struct tm* timeInfo;
	timeInfo = gmtime(&tt);
	time->year = timeInfo->tm_year + 1900;
	time->month = timeInfo->tm_mon+1;
	time->day = timeInfo->tm_mday;
	time->hour = timeInfo->tm_hour;
	time->minute = timeInfo->tm_min;
	time->second = timeInfo->tm_sec;
	auto usecond = std::chrono::duration_cast<std::chrono::microseconds>(fraction);
	time->usecond = usecond.count();

	//sif::warning << "Clock::getDateAndTime: not implemented yet" << std::endl;
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
	sif::warning << "Clock::convertTimeBla: not implemented yet" << std::endl;
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
	if(timeMutex == nullptr){
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
	if(timeMutex == nullptr){
		MutexFactory* mutexFactory = MutexFactory::instance();
		if (mutexFactory == nullptr) {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
		timeMutex = mutexFactory->createMutex();
		if (timeMutex == nullptr) {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
	}
	return HasReturnvaluesIF::RETURN_OK;
}

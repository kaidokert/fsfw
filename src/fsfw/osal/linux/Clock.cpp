#include "fsfw/timemanager/Clock.h"

#include <linux/sysinfo.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#include <ctime>
#include <fstream>

#include "fsfw/ipc/MutexGuard.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

uint32_t Clock::getTicksPerSecond() {
  uint32_t ticks = sysconf(_SC_CLK_TCK);
  return ticks;
}

ReturnValue_t Clock::setClock(const TimeOfDay_t* time) {
  timespec timeUnix{};
  timeval timeTimeval{};
  convertTimeOfDayToTimeval(time, &timeTimeval);
  timeUnix.tv_sec = timeTimeval.tv_sec;
  timeUnix.tv_nsec = (__syscall_slong_t)timeTimeval.tv_usec * 1000;

  int status = clock_settime(CLOCK_REALTIME, &timeUnix);
  if (status != 0) {
    // TODO errno
    return returnvalue::FAILED;
  }
  return returnvalue::OK;
}

ReturnValue_t Clock::setClock(const timeval* time) {
  timespec timeUnix{};
  timeUnix.tv_sec = time->tv_sec;
  timeUnix.tv_nsec = (__syscall_slong_t)time->tv_usec * 1000;
  int status = clock_settime(CLOCK_REALTIME, &timeUnix);
  if (status != 0) {
    // TODO errno
    return returnvalue::FAILED;
  }
  return returnvalue::OK;
}

ReturnValue_t Clock::getClock_timeval(timeval* time) {
  timespec timeUnix{};
  int status = clock_gettime(CLOCK_REALTIME, &timeUnix);
  if (status != 0) {
    return returnvalue::FAILED;
  }
  time->tv_sec = timeUnix.tv_sec;
  time->tv_usec = timeUnix.tv_nsec / 1000.0;
  return returnvalue::OK;
}

ReturnValue_t Clock::getClock_usecs(uint64_t* time) {
  timeval timeVal{};
  ReturnValue_t result = getClock_timeval(&timeVal);
  if (result != returnvalue::OK) {
    return result;
  }
  *time = static_cast<uint64_t>(timeVal.tv_sec) * 1e6 + timeVal.tv_usec;

  return returnvalue::OK;
}

timeval Clock::getUptime() {
  timeval uptime{};
  auto result = getUptime(&uptime);
  if (result != returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "Clock::getUptime: Error getting uptime" << std::endl;
#endif
  }
  return uptime;
}

ReturnValue_t Clock::getUptime(timeval* uptime) {
  // TODO This is not posix compatible and delivers only seconds precision
  //  Linux specific file read but more precise.
  double uptimeSeconds;
  if (std::ifstream("/proc/uptime", std::ios::in) >> uptimeSeconds) {
    uptime->tv_sec = uptimeSeconds;
    uptime->tv_usec = uptimeSeconds * (double)1e6 - (uptime->tv_sec * 1e6);
  }
  return returnvalue::OK;
}

// Wait for new FSFW Clock function delivering seconds uptime.
// uint32_t Clock::getUptimeSeconds() {
//	//TODO This is not posix compatible and delivers only seconds precision
//	struct sysinfo sysInfo;
//	int result = sysinfo(&sysInfo);
//	if(result != 0){
//		return returnvalue::FAILED;
//	}
//	return sysInfo.uptime;
//}

ReturnValue_t Clock::getUptime(uint32_t* uptimeMs) {
  timeval uptime{};
  ReturnValue_t result = getUptime(&uptime);
  if (result != returnvalue::OK) {
    return result;
  }
  *uptimeMs = uptime.tv_sec * 1e3 + uptime.tv_usec / 1e3;
  return returnvalue::OK;
}

ReturnValue_t Clock::getDateAndTime(TimeOfDay_t* time) {
  timespec timeUnix{};
  int status = clock_gettime(CLOCK_REALTIME, &timeUnix);
  if (status != 0) {
    // TODO errno
    return returnvalue::FAILED;
  }
  ReturnValue_t result = checkOrCreateClockMutex();
  if (result != returnvalue::OK) {
    return result;
  }
  MutexGuard helper(timeMutex);
  // gmtime writes its output in a global buffer which is not Thread Safe
  // Therefore we have to use a Mutex here
  struct std::tm* timeInfo;
  timeInfo = gmtime(&timeUnix.tv_sec);
  time->year = timeInfo->tm_year + 1900;
  time->month = timeInfo->tm_mon + 1;
  time->day = timeInfo->tm_mday;
  time->hour = timeInfo->tm_hour;
  time->minute = timeInfo->tm_min;
  time->second = timeInfo->tm_sec;
  time->usecond = timeUnix.tv_nsec / 1000.0;

  return returnvalue::OK;
}

ReturnValue_t Clock::convertTimeOfDayToTimeval(const TimeOfDay_t* from, timeval* to) {
  std::tm fromTm{};
  // Note: Fails for years before AD
  fromTm.tm_year = from->year - 1900;
  fromTm.tm_mon = from->month - 1;
  fromTm.tm_mday = from->day;
  fromTm.tm_hour = from->hour;
  fromTm.tm_min = from->minute;
  fromTm.tm_sec = from->second;
  fromTm.tm_isdst = 0;

  to->tv_sec = timegm(&fromTm);
  to->tv_usec = from->usecond;
  return returnvalue::OK;
}

ReturnValue_t Clock::convertTimevalToJD2000(timeval time, double* JD2000) {
  *JD2000 = (time.tv_sec - 946728000. + time.tv_usec / 1000000.) / 24. / 3600.;
  return returnvalue::OK;
}

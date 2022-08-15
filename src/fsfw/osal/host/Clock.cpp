#include "fsfw/timemanager/Clock.h"

#include <chrono>

#include "fsfw/ipc/MutexGuard.h"
#include "fsfw/platform.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

#if defined(PLATFORM_WIN)
#include <sysinfoapi.h>
#elif defined(PLATFORM_UNIX)
#include <fstream>
#endif

using SystemClock = std::chrono::system_clock;

uint32_t Clock::getTicksPerSecond(void) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::warning << "Clock::getTicksPerSecond: Not implemented for host OSAL" << std::endl;
#else
  sif::printWarning("Clock::getTicksPerSecond: Not implemented for host OSAL\n");
#endif
  /* To avoid division by zero */
  return 1;
}

ReturnValue_t Clock::setClock(const TimeOfDay_t* time) {
  /* I don't know why someone would need to set a clock which is probably perfectly fine on a
  host system with internet access so this is not implemented for now. */
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::warning << "Clock::setClock: Not implemented for host OSAL" << std::endl;
#else
  sif::printWarning("Clock::setClock: Not implemented for host OSAL\n");
#endif
  return returnvalue::OK;
}

ReturnValue_t Clock::setClock(const timeval* time) {
  /* I don't know why someone would need to set a clock which is probably perfectly fine on a
  host system with internet access so this is not implemented for now. */
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::warning << "Clock::setClock: Not implemented for host OSAL" << std::endl;
#else
  sif::printWarning("Clock::setClock: Not implemented for host OSAL\n");
#endif
  return returnvalue::OK;
}

ReturnValue_t Clock::getClock_timeval(timeval* time) {
#if defined(PLATFORM_WIN)
  auto now = std::chrono::system_clock::now();
  auto secondsChrono = std::chrono::time_point_cast<std::chrono::seconds>(now);
  auto epoch = now.time_since_epoch();
  time->tv_sec = std::chrono::duration_cast<std::chrono::seconds>(epoch).count();
  auto fraction = now - secondsChrono;
  time->tv_usec = std::chrono::duration_cast<std::chrono::microseconds>(fraction).count();
  return returnvalue::OK;
#elif defined(PLATFORM_UNIX)
  timespec timeUnix;
  int status = clock_gettime(CLOCK_REALTIME, &timeUnix);
  if (status != 0) {
    return returnvalue::FAILED;
  }
  time->tv_sec = timeUnix.tv_sec;
  time->tv_usec = timeUnix.tv_nsec / 1000.0;
  return returnvalue::OK;
#else
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::warning << "Clock::getUptime: Not implemented for found OS!" << std::endl;
#else
  sif::printWarning("Clock::getUptime: Not implemented for found OS!\n");
#endif
  return returnvalue::FAILED;
#endif
}

ReturnValue_t Clock::getClock_usecs(uint64_t* time) {
  if (time == nullptr) {
    return returnvalue::FAILED;
  }
  using namespace std::chrono;
  *time = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
  return returnvalue::OK;
}

timeval Clock::getUptime() {
  timeval timeval;
#if defined(PLATFORM_WIN)
  auto uptime = std::chrono::milliseconds(GetTickCount64());
  auto secondsChrono = std::chrono::duration_cast<std::chrono::seconds>(uptime);
  timeval.tv_sec = secondsChrono.count();
  auto fraction = uptime - secondsChrono;
  timeval.tv_usec = std::chrono::duration_cast<std::chrono::microseconds>(fraction).count();
#elif defined(PLATFORM_UNIX)
  double uptimeSeconds;
  if (std::ifstream("/proc/uptime", std::ios::in) >> uptimeSeconds) {
    // value is rounded down automatically
    timeval.tv_sec = uptimeSeconds;
    timeval.tv_usec = uptimeSeconds * (double)1e6 - (timeval.tv_sec * 1e6);
  }
#else
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::warning << "Clock::getUptime: Not implemented for found OS" << std::endl;
#endif
#endif
  return timeval;
}

ReturnValue_t Clock::getUptime(timeval* uptime) {
  *uptime = getUptime();
  return returnvalue::OK;
}

ReturnValue_t Clock::getUptime(uint32_t* uptimeMs) {
  timeval uptime = getUptime();
  *uptimeMs = uptime.tv_sec * 1000 + uptime.tv_usec / 1000;
  return returnvalue::OK;
}

ReturnValue_t Clock::getDateAndTime(TimeOfDay_t* time) {
  /* Do some magic with chrono (C++20!) */
  /* Right now, the library doesn't have the new features to get the required values yet.
  so we work around that for now. */
  auto now = SystemClock::now();
  auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
  auto fraction = now - seconds;
  time_t tt = SystemClock::to_time_t(now);
  ReturnValue_t result = checkOrCreateClockMutex();
  if (result != returnvalue::OK) {
    return result;
  }
  MutexGuard helper(timeMutex);
  // gmtime writes its output in a global buffer which is not Thread Safe
  // Therefore we have to use a Mutex here
  struct tm* timeInfo;
  timeInfo = gmtime(&tt);
  time->year = timeInfo->tm_year + 1900;
  time->month = timeInfo->tm_mon + 1;
  time->day = timeInfo->tm_mday;
  time->hour = timeInfo->tm_hour;
  time->minute = timeInfo->tm_min;
  time->second = timeInfo->tm_sec;
  auto usecond = std::chrono::duration_cast<std::chrono::microseconds>(fraction);
  time->usecond = usecond.count();
  return returnvalue::OK;
}

ReturnValue_t Clock::convertTimeOfDayToTimeval(const TimeOfDay_t* from, timeval* to) {
  struct tm time_tm {};

  time_tm.tm_year = from->year - 1900;
  time_tm.tm_mon = from->month - 1;
  time_tm.tm_mday = from->day;

  time_tm.tm_hour = from->hour;
  time_tm.tm_min = from->minute;
  time_tm.tm_sec = from->second;
  time_tm.tm_isdst = 0;

  time_t seconds = timegm(&time_tm);

  to->tv_sec = seconds;
  to->tv_usec = from->usecond;
  // Fails in 2038..
  return returnvalue::OK;
}

ReturnValue_t Clock::convertTimevalToJD2000(timeval time, double* JD2000) {
  *JD2000 = (time.tv_sec - 946728000. + time.tv_usec / 1000000.) / 24. / 3600.;
  return returnvalue::OK;
}

#include "fsfw/timemanager/Clock.h"

#include <cstdlib>
#include <ctime>

#include "FreeRTOS.h"
#include "fsfw/globalfunctions/timevalOperations.h"
#include "fsfw/osal/freertos/Timekeeper.h"
#include "task.h"

// TODO sanitize input?
// TODO much of this code can be reused for tick-only systems

uint32_t Clock::getTicksPerSecond(void) { return 1000; }

ReturnValue_t Clock::setClock(const TimeOfDay_t* time) {
  timeval time_timeval;

  ReturnValue_t result = convertTimeOfDayToTimeval(time, &time_timeval);
  if (result != returnvalue::OK) {
    return result;
  }

  return setClock(&time_timeval);
}

ReturnValue_t Clock::setClock(const timeval* time) {
  timeval uptime = getUptime();

  timeval offset = *time - uptime;

  Timekeeper::instance()->setOffset(offset);

  return returnvalue::OK;
}

ReturnValue_t Clock::getClock_timeval(timeval* time) {
  timeval uptime = getUptime();

  timeval offset = Timekeeper::instance()->getOffset();

  *time = offset + uptime;

  return returnvalue::OK;
}

ReturnValue_t Clock::getUptime(timeval* uptime) {
  *uptime = getUptime();

  return returnvalue::OK;
}

timeval Clock::getUptime() {
  TickType_t ticksSinceStart = xTaskGetTickCount();
  return Timekeeper::ticksToTimeval(ticksSinceStart);
}

ReturnValue_t Clock::getUptime(uint32_t* uptimeMs) {
  timeval uptime = getUptime();
  *uptimeMs = uptime.tv_sec * 1000 + uptime.tv_usec / 1000;
  return returnvalue::OK;
}

// uint32_t Clock::getUptimeSeconds() {
//	timeval uptime = getUptime();
//	return uptime.tv_sec;
// }

ReturnValue_t Clock::getClock_usecs(uint64_t* time) {
  timeval time_timeval;
  ReturnValue_t result = getClock_timeval(&time_timeval);
  if (result != returnvalue::OK) {
    return result;
  }
  *time = time_timeval.tv_sec * 1000000 + time_timeval.tv_usec;
  return returnvalue::OK;
}

ReturnValue_t Clock::getDateAndTime(TimeOfDay_t* time) {
  timeval time_timeval;
  ReturnValue_t result = getClock_timeval(&time_timeval);
  if (result != returnvalue::OK) {
    return result;
  }
  struct tm time_tm;

  gmtime_r(&time_timeval.tv_sec, &time_tm);

  time->year = time_tm.tm_year + 1900;
  time->month = time_tm.tm_mon + 1;
  time->day = time_tm.tm_mday;

  time->hour = time_tm.tm_hour;
  time->minute = time_tm.tm_min;
  time->second = time_tm.tm_sec;

  time->usecond = time_timeval.tv_usec;

  return returnvalue::OK;
}

ReturnValue_t Clock::convertTimeOfDayToTimeval(const TimeOfDay_t* from, timeval* to) {
  struct tm time_tm = {};

  time_tm.tm_year = from->year - 1900;
  time_tm.tm_mon = from->month - 1;
  time_tm.tm_mday = from->day;

  time_tm.tm_hour = from->hour;
  time_tm.tm_min = from->minute;
  time_tm.tm_sec = from->second;

  time_t seconds = mktime(&time_tm);

  to->tv_sec = seconds;
  to->tv_usec = from->usecond;
  // Fails in 2038..
  return returnvalue::OK;
}

ReturnValue_t Clock::convertTimevalToJD2000(timeval time, double* JD2000) {
  *JD2000 = (time.tv_sec - 946728000. + time.tv_usec / 1000000.) / 24. / 3600.;
  return returnvalue::OK;
}

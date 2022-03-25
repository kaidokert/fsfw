#include <ctime>

#include "fsfw/ipc/MutexGuard.h"
#include "fsfw/timemanager/Clock.h"

ReturnValue_t Clock::convertUTCToTT(timeval utc, timeval* tt) {
  uint16_t leapSeconds;
  ReturnValue_t result = getLeapSeconds(&leapSeconds);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  timeval leapSeconds_timeval = {0, 0};
  leapSeconds_timeval.tv_sec = leapSeconds;

  // initial offset between UTC and TAI
  timeval UTCtoTAI1972 = {10, 0};

  timeval TAItoTT = {32, 184000};

  *tt = utc + leapSeconds_timeval + UTCtoTAI1972 + TAItoTT;

  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::setLeapSeconds(const uint16_t leapSeconds_) {
  if (checkOrCreateClockMutex() != HasReturnvaluesIF::RETURN_OK) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  MutexGuard helper(timeMutex);

  leapSeconds = leapSeconds_;

  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::getLeapSeconds(uint16_t* leapSeconds_) {
  if (timeMutex == nullptr) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  MutexGuard helper(timeMutex);

  *leapSeconds_ = leapSeconds;

  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::convertTimevalToTimeOfDay(const timeval* from, TimeOfDay_t* to) {
  struct tm* timeInfo;
  // According to https://en.cppreference.com/w/c/chrono/gmtime, the implementation of gmtime_s
  // in the Windows CRT is incompatible with the C standard but this should not be an issue for
  // this implementation
  timeInfo = gmtime(&from->tv_sec);
  to->year = timeInfo->tm_year + 1900;
  to->month = timeInfo->tm_mon + 1;
  to->day = timeInfo->tm_mday;
  to->hour = timeInfo->tm_hour;
  to->minute = timeInfo->tm_min;
  to->second = timeInfo->tm_sec;
  to->usecond = from->tv_usec;
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t Clock::checkOrCreateClockMutex() {
  if (timeMutex == nullptr) {
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

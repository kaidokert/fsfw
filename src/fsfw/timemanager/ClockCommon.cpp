#include <ctime>

#include "fsfw/ipc/MutexGuard.h"
#include "fsfw/timemanager/Clock.h"

uint16_t Clock::leapSeconds = 0;
MutexIF* Clock::timeMutex = nullptr;
bool Clock::leapSecondsSet = false;

ReturnValue_t Clock::convertUTCToTT(timeval utc, timeval* tt) {
  uint16_t leapSeconds;
  ReturnValue_t result = getLeapSeconds(&leapSeconds);
  if (result != returnvalue::OK) {
    return result;
  }
  timeval leapSeconds_timeval = {0, 0};
  leapSeconds_timeval.tv_sec = leapSeconds;

  // initial offset between UTC and TAI
  timeval UTCtoTAI1972 = {10, 0};

  timeval TAItoTT = {32, 184000};

  *tt = utc + leapSeconds_timeval + UTCtoTAI1972 + TAItoTT;

  return returnvalue::OK;
}

ReturnValue_t Clock::setLeapSeconds(const uint16_t leapSeconds_) {
  if (checkOrCreateClockMutex() != returnvalue::OK) {
    return returnvalue::FAILED;
  }
  MutexGuard helper(timeMutex);

  leapSeconds = leapSeconds_;
  leapSecondsSet = true;

  return returnvalue::OK;
}

ReturnValue_t Clock::getLeapSeconds(uint16_t* leapSeconds_) {
  if (not leapSecondsSet) {
    return returnvalue::FAILED;
  }
  if (checkOrCreateClockMutex() != returnvalue::OK) {
    return returnvalue::FAILED;
  }
  MutexGuard helper(timeMutex);

  *leapSeconds_ = leapSeconds;

  return returnvalue::OK;
}

ReturnValue_t Clock::convertTimevalToTimeOfDay(const timeval* from, TimeOfDay_t* to) {
  struct tm* timeInfo;
  // According to https://en.cppreference.com/w/c/chrono/gmtime, the implementation of gmtime_s
  // in the Windows CRT is incompatible with the C standard but this should not be an issue for
  // this implementation
  ReturnValue_t result = checkOrCreateClockMutex();
  if (result != returnvalue::OK) {
    return result;
  }
  MutexGuard helper(timeMutex);
  // gmtime writes its output in a global buffer which is not Thread Safe
  // Therefore we have to use a Mutex here
  timeInfo = gmtime(&from->tv_sec);
  to->year = timeInfo->tm_year + 1900;
  to->month = timeInfo->tm_mon + 1;
  to->day = timeInfo->tm_mday;
  to->hour = timeInfo->tm_hour;
  to->minute = timeInfo->tm_min;
  to->second = timeInfo->tm_sec;
  to->usecond = from->tv_usec;
  return returnvalue::OK;
}

ReturnValue_t Clock::checkOrCreateClockMutex() {
  if (timeMutex == nullptr) {
    MutexFactory* mutexFactory = MutexFactory::instance();
    if (mutexFactory == nullptr) {
      return returnvalue::FAILED;
    }
    timeMutex = mutexFactory->createMutex();
    if (timeMutex == nullptr) {
      return returnvalue::FAILED;
    }
  }
  return returnvalue::OK;
}

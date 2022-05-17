#include "fsfw/timemanager/Countdown.h"

Countdown::Countdown(uint32_t initialTimeout) : timeout(initialTimeout) {
  setTimeout(initialTimeout);
}

Countdown::~Countdown() {}

ReturnValue_t Countdown::setTimeout(uint32_t milliseconds) {
  ReturnValue_t returnValue = Clock::getUptime(&startTime);
  timeout = milliseconds;
  return returnValue;
}

bool Countdown::hasTimedOut() const {
  if (uint32_t(this->getCurrentTime() - startTime) >= timeout) {
    return true;
  } else {
    return false;
  }
}

bool Countdown::isBusy() const { return !hasTimedOut(); }

ReturnValue_t Countdown::resetTimer() { return setTimeout(timeout); }

void Countdown::timeOut() { startTime = this->getCurrentTime() - timeout; }

uint32_t Countdown::getRemainingMillis() const {
  // We fetch the time before the if-statement
  // to be sure that the return is in
  // range 0 <= number <= timeout
  uint32_t currentTime = this->getCurrentTime();
  if (this->hasTimedOut()) {
    return 0;
  } else {
    return (startTime + timeout) - currentTime;
  }
}

uint32_t Countdown::getCurrentTime() const {
  uint32_t currentTime;
  Clock::getUptime(&currentTime);
  return currentTime;
}

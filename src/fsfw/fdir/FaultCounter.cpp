#include "fsfw/fdir/FaultCounter.h"

FaultCounter::FaultCounter(uint32_t failureThreshold, uint32_t decrementAfterMs,
                           uint8_t setParameterDomain)
    : parameterDomain(setParameterDomain),
      timer(),
      faultCount(0),
      failureThreshold(failureThreshold) {
  timer.setTimeout(decrementAfterMs);
}

FaultCounter::~FaultCounter() {}

void FaultCounter::increment(uint32_t amount) {
  if (faultCount == 0) {
    timer.resetTimer();
  }
  faultCount += amount;
}

bool FaultCounter::checkForDecrement() {
  if (timer.hasTimedOut()) {
    timer.resetTimer();
    if (faultCount > 0) {
      faultCount--;
      return true;
    }
  }
  return false;
}

bool FaultCounter::incrementAndCheck(uint32_t amount) {
  increment(amount);
  return aboveThreshold();
}

bool FaultCounter::aboveThreshold() {
  if (faultCount > failureThreshold) {
    faultCount = 0;
    return true;
  } else {
    return false;
  }
}

void FaultCounter::clear() { faultCount = 0; }

void FaultCounter::setFailureThreshold(uint32_t failureThreshold) {
  this->failureThreshold = failureThreshold;
}

void FaultCounter::setFaultDecrementTimeMs(uint32_t timeMs) { timer.setTimeout(timeMs); }

FaultCounter::FaultCounter() : parameterDomain(0), timer(), faultCount(0), failureThreshold(0) {}

ReturnValue_t FaultCounter::getParameter(uint8_t domainId, uint8_t uniqueId,
                                         ParameterWrapper* parameterWrapper,
                                         const ParameterWrapper* newValues, uint16_t startAtIndex) {
  if (domainId != parameterDomain) {
    return INVALID_DOMAIN_ID;
  }

  switch (static_cast<ParameterIds>(uniqueId)) {
    case ParameterIds::FAILURE_THRESHOLD:
      parameterWrapper->set(failureThreshold);
      break;
    case ParameterIds::FAULT_COUNT:
      parameterWrapper->set(faultCount);
      break;
    case ParameterIds::TIMEOUT:
      parameterWrapper->set(timer.timeout);
      break;
    default:
      return INVALID_IDENTIFIER_ID;
  }
  return returnvalue::OK;
}

void FaultCounter::setParameterDomain(uint8_t domain) { parameterDomain = domain; }

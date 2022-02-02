#include "fsfw/fdir/EventCorrelation.h"

EventCorrelation::EventCorrelation(uint32_t timeout) : eventPending(false) {
  correlationTimer.setTimeout(timeout);
}

EventCorrelation::~EventCorrelation() {}

EventCorrelation::State EventCorrelation::doesEventCorrelate() {
  if (correlationTimer.isBusy()) {
    eventPending = false;
    return CORRELATED;
  } else {
    if (eventPending) {
      return ALREADY_STARTED;
    } else {
      eventPending = true;
      correlationTimer.resetTimer();
      return CORRELATION_STARTED;
    }
  }
}

bool EventCorrelation::isEventPending() {
  if (eventPending) {
    eventPending = false;
    return true;
  } else {
    correlationTimer.resetTimer();
    return false;
  }
}

bool EventCorrelation::hasPendingEventTimedOut() {
  if (correlationTimer.hasTimedOut()) {
    bool temp = eventPending;
    eventPending = false;
    return temp;
  } else {
    return false;
  }
}

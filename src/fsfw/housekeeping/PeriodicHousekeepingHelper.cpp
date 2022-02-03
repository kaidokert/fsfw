#include "fsfw/housekeeping/PeriodicHousekeepingHelper.h"

#include <cmath>

#include "fsfw/datapoollocal/LocalPoolDataSetBase.h"

PeriodicHousekeepingHelper::PeriodicHousekeepingHelper(LocalPoolDataSetBase* owner)
    : owner(owner) {}

void PeriodicHousekeepingHelper::initialize(float collectionInterval,
                                            dur_millis_t minimumPeriodicInterval,
                                            uint8_t nonDiagIntervalFactor) {
  this->minimumPeriodicInterval = minimumPeriodicInterval;
  this->nonDiagIntervalFactor = nonDiagIntervalFactor;
  collectionIntervalTicks = intervalSecondsToIntervalTicks(collectionInterval);
  /* This will cause a checkOpNecessary call to be true immediately. I think it's okay
      if a HK packet is generated immediately instead of waiting one generation cycle. */
  internalTickCounter = collectionIntervalTicks;
}

float PeriodicHousekeepingHelper::getCollectionIntervalInSeconds() const {
  return intervalTicksToSeconds(collectionIntervalTicks);
}

bool PeriodicHousekeepingHelper::checkOpNecessary() {
  if (internalTickCounter >= collectionIntervalTicks) {
    internalTickCounter = 1;
    return true;
  }
  internalTickCounter++;
  return false;
}

uint32_t PeriodicHousekeepingHelper::intervalSecondsToIntervalTicks(
    float collectionIntervalSeconds) {
  if (owner == nullptr) {
    return 0;
  }
  bool isDiagnostics = owner->isDiagnostics();

  /* Avoid division by zero */
  if (minimumPeriodicInterval == 0) {
    if (isDiagnostics) {
      /* Perform operation each cycle */
      return 1;
    } else {
      return nonDiagIntervalFactor;
    }
  } else {
    dur_millis_t intervalInMs = collectionIntervalSeconds * 1000;
    uint32_t divisor = minimumPeriodicInterval;
    if (not isDiagnostics) {
      /* We need to multiply the divisor because non-diagnostics only
      allow a multiple of the minimum periodic interval */
      divisor *= nonDiagIntervalFactor;
    }
    uint32_t ticks = std::ceil(static_cast<float>(intervalInMs) / divisor);
    if (not isDiagnostics) {
      /* Now we need to multiply the calculated ticks with the factor as as well
      because the minimum tick count to generate a non-diagnostic is the factor itself.

      Example calculation for non-diagnostic with
      0.4 second interval and 0.2 second task interval.
      Resultant tick count of 5 is equal to operation each second.

      Examle calculation for non-diagnostic with  2.0 second interval and 0.2 second
      task interval.
      Resultant tick count of 10 is equal to operatin every 2 seconds.

      Example calculation for diagnostic with 0.4 second interval and 0.3
      second task interval. Resulting tick count of 2 is equal to operation
      every 0.6 seconds. */
      ticks *= nonDiagIntervalFactor;
    }
    return ticks;
  }
}

float PeriodicHousekeepingHelper::intervalTicksToSeconds(uint32_t collectionInterval) const {
  /* Number of ticks times the minimum interval is in milliseconds, so we divide by 1000 to get
  the value in seconds */
  return static_cast<float>(collectionInterval * minimumPeriodicInterval / 1000.0);
}

void PeriodicHousekeepingHelper::changeCollectionInterval(float newIntervalSeconds) {
  collectionIntervalTicks = intervalSecondsToIntervalTicks(newIntervalSeconds);
}

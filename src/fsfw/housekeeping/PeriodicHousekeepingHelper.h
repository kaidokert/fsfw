#ifndef FSFW_HOUSEKEEPING_PERIODICHOUSEKEEPINGHELPER_H_
#define FSFW_HOUSEKEEPING_PERIODICHOUSEKEEPINGHELPER_H_

#include <cstdint>

#include "fsfw/timemanager/Clock.h"

class LocalPoolDataSetBase;

class PeriodicHousekeepingHelper {
 public:
  PeriodicHousekeepingHelper(LocalPoolDataSetBase* owner);

  void initialize(float collectionInterval, dur_millis_t minimumPeriodicInterval,
                  uint8_t nonDiagIntervalFactor);

  void changeCollectionInterval(float newInterval);
  float getCollectionIntervalInSeconds() const;
  bool checkOpNecessary();

 private:
  LocalPoolDataSetBase* owner = nullptr;
  uint8_t nonDiagIntervalFactor = 0;

  uint32_t intervalSecondsToIntervalTicks(float collectionIntervalSeconds);
  float intervalTicksToSeconds(uint32_t collectionInterval) const;

  dur_millis_t minimumPeriodicInterval = 0;
  uint32_t internalTickCounter = 1;
  uint32_t collectionIntervalTicks = 0;
};

#endif /* FSFW_HOUSEKEEPING_PERIODICHOUSEKEEPINGHELPER_H_ */

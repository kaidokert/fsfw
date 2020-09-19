#ifndef FSFW_HOUSEKEEPING_PERIODICHOUSEKEEPINGHELPER_H_
#define FSFW_HOUSEKEEPING_PERIODICHOUSEKEEPINGHELPER_H_

#include "../timemanager/Clock.h"
#include <cstdint>

class LocalPoolDataSetBase;

class PeriodicHousekeepingHelper {
public:
	PeriodicHousekeepingHelper(LocalPoolDataSetBase* owner);

	void initialize(float collectionInterval,
			dur_millis_t minimumPeriodicInterval, bool isDiagnostics,
			uint8_t nonDiagIntervalFactor);

	void changeCollectionInterval(float newInterval);
	float getCollectionIntervalInSeconds();
	bool checkOpNecessary();
private:
	LocalPoolDataSetBase* owner = nullptr;


	uint32_t intervalSecondsToInterval(float collectionIntervalSeconds);
	float intervalToIntervalSeconds(uint32_t collectionInterval);

	dur_millis_t minimumPeriodicInterval = 0;
	uint32_t internalTickCounter = 1;
	uint32_t collectionIntervalTicks = 0;

};



#endif /* FSFW_HOUSEKEEPING_PERIODICHOUSEKEEPINGHELPER_H_ */

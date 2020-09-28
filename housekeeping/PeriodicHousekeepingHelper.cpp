#include "../datapoollocal/LocalPoolDataSetBase.h"
#include "PeriodicHousekeepingHelper.h"
#include <cmath>

PeriodicHousekeepingHelper::PeriodicHousekeepingHelper(
		LocalPoolDataSetBase* owner): owner(owner) {}


void PeriodicHousekeepingHelper::initialize(float collectionInterval,
		dur_millis_t minimumPeriodicInterval, bool isDiagnostics,
		uint8_t nonDiagIntervalFactor) {
	this->minimumPeriodicInterval = minimumPeriodicInterval;
	if(not isDiagnostics) {
		this->minimumPeriodicInterval = this->minimumPeriodicInterval *
				nonDiagIntervalFactor;
	}
	collectionIntervalTicks = intervalSecondsToInterval(collectionInterval);
}

float PeriodicHousekeepingHelper::getCollectionIntervalInSeconds() {
	return intervalToIntervalSeconds(collectionIntervalTicks);
}

bool PeriodicHousekeepingHelper::checkOpNecessary() {
	if(internalTickCounter >= collectionIntervalTicks) {
		internalTickCounter = 1;
		return true;
	}
	internalTickCounter++;
	return false;
}

uint32_t PeriodicHousekeepingHelper::intervalSecondsToInterval(
		float collectionIntervalSeconds) {
        return std::ceil(collectionIntervalSeconds * 1000
                / minimumPeriodicInterval);
}

float PeriodicHousekeepingHelper::intervalToIntervalSeconds(
		uint32_t collectionInterval) {
	return static_cast<float>(collectionInterval *
			minimumPeriodicInterval);
}

void PeriodicHousekeepingHelper::changeCollectionInterval(
		float newIntervalSeconds) {
	collectionIntervalTicks = intervalSecondsToInterval(newIntervalSeconds);
}

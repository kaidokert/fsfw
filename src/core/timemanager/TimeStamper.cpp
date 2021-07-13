#include "TimeStamper.h"
#include "Clock.h"
#include <cstring>

TimeStamper::TimeStamper(object_id_t objectId): SystemObject(objectId) {}


ReturnValue_t TimeStamper::addTimeStamp(uint8_t* buffer,
		const uint8_t maxSize) {
	if(maxSize < TimeStamperIF::MISSION_TIMESTAMP_SIZE){
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	timeval now;
	Clock::getClock_timeval(&now);
	CCSDSTime::CDS_short cds;
	ReturnValue_t result = CCSDSTime::convertToCcsds(&cds,&now);
	if(result != HasReturnvaluesIF::RETURN_OK){
		return result;
	}
	std::memcpy(buffer,&cds,sizeof(cds));
	return result;
}

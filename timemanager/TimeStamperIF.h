#ifndef FRAMEWORK_TIMEMANAGER_TIMESTAMPERIF_H_
#define FRAMEWORK_TIMEMANAGER_TIMESTAMPERIF_H_

#include "../returnvalues/HasReturnvaluesIF.h"

/**
 * A class implementing this IF provides facilities to add a time stamp to the
 * buffer provided.
 * Implementors need to ensure that calling the method is thread-safe, i.e.
 * addTimeStamp may be called in parallel from a different context.
 */
class TimeStamperIF {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::TIME_STAMPER_IF;
	static const ReturnValue_t BAD_TIMESTAMP = MAKE_RETURN_CODE(1);

	//! This is a mission-specific constant and determines the total
	//! size reserved for timestamps.
	//! TODO: Default define in FSFWConfig ?
	static const uint8_t MISSION_TIMESTAMP_SIZE = 8;
	virtual ReturnValue_t addTimeStamp(uint8_t* buffer,
	        const uint8_t maxSize) = 0;
	virtual ~TimeStamperIF() {}
};



#endif /* FRAMEWORK_TIMEMANAGER_TIMESTAMPERIF_H_ */

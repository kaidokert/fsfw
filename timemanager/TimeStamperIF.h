/*
 * TimeStamperIF.h
 *
 *  Created on: 31.03.2015
 *      Author: baetz
 */

#ifndef FRAMEWORK_TIMEMANAGER_TIMESTAMPERIF_H_
#define FRAMEWORK_TIMEMANAGER_TIMESTAMPERIF_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>

class TimeStamperIF {
public:
	static const uint8_t MISSION_TIMESTAMP_SIZE = 8; //!< This is a mission-specific constant and determines the total size reserved for timestamps.
	virtual ReturnValue_t addTimeStamp(uint8_t* buffer, const uint8_t maxSize) = 0;
	virtual ~TimeStamperIF() {}
};



#endif /* FRAMEWORK_TIMEMANAGER_TIMESTAMPERIF_H_ */

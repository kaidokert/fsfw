#ifndef FSFW_TIMEMANAGER_TIMESTAMPER_H_
#define FSFW_TIMEMANAGER_TIMESTAMPER_H_

#include "TimeStamperIF.h"
#include "CCSDSTime.h"
#include "../objectmanager/SystemObject.h"

/**
 * @brief   Time stamper which can be used to add any timestamp to a
 *          given buffer.
 * @details
 * This time stamper uses the CCSDS CDC short timestamp as a fault timestamp.
 * This timestamp has a size of 8 bytes. A custom timestamp can be used by
 * overriding the #addTimeStamp function.
 * @ingroup utility
 */
class TimeStamper: public TimeStamperIF, public SystemObject {
public:
    /**
     * @brief   Default constructor which also registers the time stamper as a
     *          system object so it can be found with the #objectManager.
     * @param objectId
     */
	TimeStamper(object_id_t objectId);

	/**
	 * Adds a CCSDS CDC short 8 byte timestamp to the given buffer.
	 * This function can be overriden to use a custom timestamp.
	 * @param buffer
	 * @param maxSize
	 * @return
	 */
	virtual ReturnValue_t addTimeStamp(uint8_t* buffer, const uint8_t maxSize);
};

#endif /* FSFW_TIMEMANAGER_TIMESTAMPER_H_ */

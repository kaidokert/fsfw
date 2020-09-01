/**
 * @file	AcceptsDeviceResponsesIF.h
 * @brief	This file defines the AcceptsDeviceResponsesIF class.
 * @date	15.05.2013
 * @author	baetz
 */

#ifndef ACCEPTSDEVICERESPONSESIF_H_
#define ACCEPTSDEVICERESPONSESIF_H_

#include "../ipc/MessageQueueSenderIF.h"

class AcceptsDeviceResponsesIF {
public:
	/**
	 * Default empty virtual destructor.
	 */
	virtual ~AcceptsDeviceResponsesIF() {
}
virtual MessageQueueId_t getDeviceQueue() = 0;
};

#endif /* ACCEPTSDEVICERESPONSESIF_H_ */

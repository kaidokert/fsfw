/**
 * \file AcceptsThermalMessagesIF.h
 *
 * \date 16.02.2020
 */

#ifndef FRAMEWORK_THERMAL_ACCEPTSTHERMALMESSAGESIF_H_
#define FRAMEWORK_THERMAL_ACCEPTSTHERMALMESSAGESIF_H_
#include <framework/ipc/MessageQueueSenderIF.h>

class AcceptsThermalMessagesIF {
public:
	virtual ~AcceptsThermalMessagesIF();

	virtual MessageQueueId_t getReceptionQueue() const = 0;
};

#endif /* FRAMEWORK_THERMAL_ACCEPTSTHERMALMESSAGESIF_H_ */

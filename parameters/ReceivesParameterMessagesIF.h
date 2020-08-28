#ifndef RECEIVESPARAMETERMESSAGESIF_H_
#define RECEIVESPARAMETERMESSAGESIF_H_


#include "../parameters/HasParametersIF.h"
#include "../ipc/MessageQueueSenderIF.h"

class ReceivesParameterMessagesIF : public HasParametersIF {
public:

	static const uint8_t DOMAIN_ID_BASE = 0;
	virtual ~ReceivesParameterMessagesIF() {
	}

	virtual MessageQueueId_t getCommandQueue() const = 0;
};


#endif /* RECEIVESPARAMETERMESSAGESIF_H_ */

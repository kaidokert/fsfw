#ifndef RECEIVESPARAMETERMESSAGESIF_H_
#define RECEIVESPARAMETERMESSAGESIF_H_


#include <framework/ipc/MessageQueue.h>
#include <framework/parameters/HasParametersIF.h>

class ReceivesParameterMessagesIF : public HasParametersIF {
public:

	virtual ~ReceivesParameterMessagesIF() {
	}

	virtual MessageQueueId_t getCommandQueue() const = 0;
};


#endif /* RECEIVESPARAMETERMESSAGESIF_H_ */

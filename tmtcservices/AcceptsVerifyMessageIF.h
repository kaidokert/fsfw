#ifndef ACCEPTSVERIFICATIONMESSAGEIF_H_
#define ACCEPTSVERIFICATIONMESSAGEIF_H_

#include "../ipc/MessageQueueSenderIF.h"

class AcceptsVerifyMessageIF {
public:
	virtual ~AcceptsVerifyMessageIF() {

	}
	virtual MessageQueueId_t getVerificationQueue() = 0;
};


#endif /* ACCEPTSVERIFICATIONMESSAGEIF_H_ */

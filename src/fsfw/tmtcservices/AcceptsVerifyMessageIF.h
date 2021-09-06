#ifndef FSFW_TMTCSERVICES_ACCEPTSVERIFICATIONMESSAGEIF_H_
#define FSFW_TMTCSERVICES_ACCEPTSVERIFICATIONMESSAGEIF_H_

#include "../ipc/MessageQueueSenderIF.h"

class AcceptsVerifyMessageIF {
public:
	virtual ~AcceptsVerifyMessageIF() {

	}
	virtual MessageQueueId_t getVerificationQueue() = 0;
};


#endif /* FSFW_TMTCSERVICES_ACCEPTSVERIFICATIONMESSAGEIF_H_ */

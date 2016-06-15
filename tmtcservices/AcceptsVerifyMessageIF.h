/*
 * AcceptsVerifyMessageIF.h
 *
 *  Created on: 23.11.2012
 *      Author: baetz
 */

#ifndef ACCEPTSVERIFICATIONMESSAGEIF_H_
#define ACCEPTSVERIFICATIONMESSAGEIF_H_

#include <framework/ipc/MessageQueue.h>

class AcceptsVerifyMessageIF {
public:
	virtual ~AcceptsVerifyMessageIF() {

	}
	virtual MessageQueueId_t getVerificationQueue() = 0;
};


#endif /* ACCEPTSVERIFICATIONMESSAGEIF_H_ */

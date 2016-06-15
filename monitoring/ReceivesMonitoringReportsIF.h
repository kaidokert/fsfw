/*
 * ReceivesMonitoringReportsIF.h
 *
 *  Created on: 07.07.2014
 *      Author: baetz
 */

#ifndef RECEIVESMONITORINGREPORTSIF_H_
#define RECEIVESMONITORINGREPORTSIF_H_

#include <framework/ipc/MessageQueue.h>

class ReceivesMonitoringReportsIF {
public:
	virtual MessageQueueId_t getCommandQueue() const = 0;
	virtual ~ReceivesMonitoringReportsIF() {
	}
};



#endif /* RECEIVESMONITORINGREPORTSIF_H_ */

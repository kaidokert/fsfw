#ifndef RECEIVESMONITORINGREPORTSIF_H_
#define RECEIVESMONITORINGREPORTSIF_H_

#include <framework/ipc/MessageQueueSenderIF.h>

class ReceivesMonitoringReportsIF {
public:
	virtual MessageQueueId_t getCommandQueue() const = 0;
	virtual ~ReceivesMonitoringReportsIF() {
	}
};



#endif /* RECEIVESMONITORINGREPORTSIF_H_ */

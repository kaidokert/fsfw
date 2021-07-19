#ifndef RECEIVESMONITORINGREPORTSIF_H_
#define RECEIVESMONITORINGREPORTSIF_H_

#include "monitoringConf.h"
#include "fsfw/ipc/messageQueueDefinitions.h"

class ReceivesMonitoringReportsIF {
public:
	virtual MessageQueueId_t getCommandQueue() const = 0;
	virtual ~ReceivesMonitoringReportsIF() {
	}
};



#endif /* RECEIVESMONITORINGREPORTSIF_H_ */

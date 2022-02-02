#ifndef RECEIVESMONITORINGREPORTSIF_H_
#define RECEIVESMONITORINGREPORTSIF_H_

#include "fsfw/ipc/messageQueueDefinitions.h"
#include "monitoringConf.h"

class ReceivesMonitoringReportsIF {
 public:
  virtual MessageQueueId_t getCommandQueue() const = 0;
  virtual ~ReceivesMonitoringReportsIF() {}
};

#endif /* RECEIVESMONITORINGREPORTSIF_H_ */

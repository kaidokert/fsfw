#ifndef FSFW_TMTCSERVICES_ACCEPTSVERIFICATIONMESSAGEIF_H_
#define FSFW_TMTCSERVICES_ACCEPTSVERIFICATIONMESSAGEIF_H_

#include "fsfw/ipc/MessageQueueSenderIF.h"

class AcceptsVerifyMessageIF {
 public:
  virtual ~AcceptsVerifyMessageIF() = default;
  virtual MessageQueueId_t getVerificationQueue() = 0;
};

#endif /* FSFW_TMTCSERVICES_ACCEPTSVERIFICATIONMESSAGEIF_H_ */

#ifndef FSFW_PARAMETERS_RECEIVESPARAMETERMESSAGESIF_H_
#define FSFW_PARAMETERS_RECEIVESPARAMETERMESSAGESIF_H_

#include "../ipc/MessageQueueSenderIF.h"
#include "HasParametersIF.h"

class ReceivesParameterMessagesIF : public HasParametersIF {
 public:
  static const uint8_t DOMAIN_ID_BASE = 0;
  virtual ~ReceivesParameterMessagesIF() {}

  virtual MessageQueueId_t getCommandQueue() const = 0;
};

#endif /* FSFW_PARAMETERS_RECEIVESPARAMETERMESSAGESIF_H_ */

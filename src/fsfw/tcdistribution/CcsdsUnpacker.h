#ifndef FSFW_TCDISTRIBUTION_CCSDSUNPACKER_H
#define FSFW_TCDISTRIBUTION_CCSDSUNPACKER_H

#include "fsfw/tasks/ExecutableObjectIF.h"
#include "fsfw/tmtcservices/AcceptsTelecommandsIF.h"

class CcsdsUnpacker: public ExecutableObjectIF, public AcceptsTelecommandsIF {
 public:
  CcsdsUnpacker();
  ReturnValue_t performOperation(uint8_t operationCode) override;
  uint32_t getIdentifier() override;
  MessageQueueId_t getRequestQueue() override;

 private:
};
#endif  // FSFW_TCDISTRIBUTION_CCSDSUNPACKER_H

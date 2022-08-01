#ifndef FSFW_TCDISTRIBUTION_CCSDSUNPACKER_H
#define FSFW_TCDISTRIBUTION_CCSDSUNPACKER_H

#include "fsfw/storagemanager/StorageManagerIF.h"
#include "fsfw/tasks/ExecutableObjectIF.h"
#include "fsfw/tmtcservices/AcceptsTelecommandsIF.h"

class CcsdsUnpacker : public ExecutableObjectIF, public AcceptsTelecommandsIF {
 public:
  CcsdsUnpacker(MessageQueueIF& msgQueue, AcceptsTelecommandsIF& receiver,
                StorageManagerIF& sourceStore);

  void setDifferentTargetStore(StorageManagerIF& otherTargetStore);
  ReturnValue_t performOperation(uint8_t operationCode) override;
  uint32_t getIdentifier() const override;
  MessageQueueId_t getRequestQueue() const override;

 private:
  StorageManagerIF& sourceStore;
  StorageManagerIF* targetStore = nullptr;
  MessageQueueIF& msgQueue;
  AcceptsTelecommandsIF& receiver;
};
#endif  // FSFW_TCDISTRIBUTION_CCSDSUNPACKER_H

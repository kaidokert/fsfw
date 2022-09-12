#ifndef FSFW_TMTCPACKET_TMSENDHELPER_H
#define FSFW_TMTCPACKET_TMSENDHELPER_H

#include "TmTcMessage.h"
#include "fsfw/internalerror/InternalErrorReporterIF.h"
#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/ipc/messageQueueDefinitions.h"
#include "fsfw/returnvalues/returnvalue.h"

class TmSendHelper {
 public:
  TmSendHelper();
  TmSendHelper(MessageQueueIF& queue, InternalErrorReporterIF& reporter,
               MessageQueueId_t tmtcMsgDest);
  TmSendHelper(MessageQueueIF& queue, InternalErrorReporterIF& reporter);
  explicit TmSendHelper(InternalErrorReporterIF& reporter);

  void setMsgQueue(MessageQueueIF& queue);
  [[nodiscard]] MessageQueueIF* getMsgQueue() const;

  void setDefaultDestination(MessageQueueId_t msgDest);
  [[nodiscard]] MessageQueueId_t getDefaultDestination() const;
  [[nodiscard]] bool areFaultsIgnored() const;
  void ignoreFaults();
  void dontIgnoreFaults();
  void setInternalErrorReporter(InternalErrorReporterIF& reporter);
  [[nodiscard]] InternalErrorReporterIF* getInternalErrorReporter() const;
  ReturnValue_t sendPacket(MessageQueueId_t dest, const store_address_t& storeId);
  ReturnValue_t sendPacket(const store_address_t& storeId);

 private:
  bool ignoreFault = false;
  MessageQueueIF* queue = nullptr;
  InternalErrorReporterIF* errReporter = nullptr;
};

#endif  // FSFW_TMTCPACKET_TMSENDHELPER_H

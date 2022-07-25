#ifndef FSFW_TMTCPACKET_TMSENDHELPER_H
#define FSFW_TMTCPACKET_TMSENDHELPER_H

#include "TmTcMessage.h"
#include "fsfw/internalerror/InternalErrorReporterIF.h"
#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/ipc/messageQueueDefinitions.h"
#include "fsfw/returnvalues/HasReturnvaluesIF.h"

class TmSendHelper {
 public:
  explicit TmSendHelper(InternalErrorReporterIF* reporter);
  TmSendHelper(MessageQueueIF* queue, InternalErrorReporterIF* reporter);
  TmSendHelper(MessageQueueIF* queue, MessageQueueId_t tmtcMsgDest,
               InternalErrorReporterIF* reporter);
  void setMsgQueue(MessageQueueIF* queue);
  void setMsgDestination(MessageQueueId_t msgDest);
  void setInternalErrorReporter(InternalErrorReporterIF* reporter);
  ReturnValue_t sendPacket(const store_address_t& storeId);

 private:
  MessageQueueId_t tmtcMsgDest = MessageQueueIF::NO_QUEUE;
  bool ignoreFault = false;
  MessageQueueIF* queue = nullptr;
  InternalErrorReporterIF* errReporter;
};

#endif  // FSFW_TMTCPACKET_TMSENDHELPER_H

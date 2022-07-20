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
  TmSendHelper(MessageQueueId_t tmtcMsgDest, MessageQueueId_t tmtcMsgSrc,
               InternalErrorReporterIF* reporter);
  void setMsgDestination(MessageQueueId_t msgDest);
  void setMsgSource(MessageQueueId_t msgSrc);
  void setInternalErrorReporter(InternalErrorReporterIF* reporter);
  ReturnValue_t sendPacket(const store_address_t& storeId);

 private:
  MessageQueueId_t tmtcMsgDest = MessageQueueIF::NO_QUEUE;
  MessageQueueId_t tmtcMsgSrc = MessageQueueIF::NO_QUEUE;
  InternalErrorReporterIF* errReporter;
};

#endif  // FSFW_TMTCPACKET_TMSENDHELPER_H

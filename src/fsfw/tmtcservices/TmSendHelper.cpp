#include "TmSendHelper.h"

#include "fsfw/ipc/MessageQueueSenderIF.h"

TmSendHelper::TmSendHelper(MessageQueueId_t tmtcMsgDest, MessageQueueId_t tmtcMsgSrc,
                           InternalErrorReporterIF *reporter)
    : tmtcMsgDest(tmtcMsgDest), tmtcMsgSrc(tmtcMsgSrc), errReporter(reporter) {}

TmSendHelper::TmSendHelper(InternalErrorReporterIF *reporter) : errReporter(reporter) {}

ReturnValue_t TmSendHelper::sendPacket(const store_address_t &storeId) {
  TmTcMessage message(storeId);
  ReturnValue_t result = MessageQueueSenderIF::sendMessage(tmtcMsgDest, &message, tmtcMsgSrc);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    if (errReporter != nullptr) {
      errReporter->lostTm();
    }
    return result;
  }
  return HasReturnvaluesIF::RETURN_OK;
}

void TmSendHelper::setMsgDestination(MessageQueueId_t msgDest) { tmtcMsgDest = msgDest; }

void TmSendHelper::setMsgSource(MessageQueueId_t msgSrc) { tmtcMsgSrc = msgSrc; }

void TmSendHelper::setInternalErrorReporter(InternalErrorReporterIF *reporter) {
  errReporter = reporter;
}

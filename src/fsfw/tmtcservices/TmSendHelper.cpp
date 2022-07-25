#include "TmSendHelper.h"

#include "fsfw/ipc/MessageQueueSenderIF.h"

TmSendHelper::TmSendHelper(MessageQueueIF *queue, InternalErrorReporterIF *reporter,
                           MessageQueueId_t tmtcMsgDest)
    : tmtcMsgDest(tmtcMsgDest), queue(queue), errReporter(reporter) {}

TmSendHelper::TmSendHelper(MessageQueueIF *queue, InternalErrorReporterIF *reporter)
    : queue(queue), errReporter(reporter) {}

TmSendHelper::TmSendHelper(InternalErrorReporterIF *reporter) : errReporter(reporter) {}

ReturnValue_t TmSendHelper::sendPacket(const store_address_t &storeId) {
  if (queue == nullptr) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  TmTcMessage message(storeId);
  ReturnValue_t result = queue->sendMessage(tmtcMsgDest, &message, ignoreFault);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    if (errReporter != nullptr) {
      errReporter->lostTm();
    }
    return result;
  }
  return HasReturnvaluesIF::RETURN_OK;
}

void TmSendHelper::setMsgDestination(MessageQueueId_t msgDest) { tmtcMsgDest = msgDest; }

void TmSendHelper::setInternalErrorReporter(InternalErrorReporterIF *reporter) {
  errReporter = reporter;
}
void TmSendHelper::setMsgQueue(MessageQueueIF *queue_) { queue = queue_; }

ReturnValue_t TmSendHelper::sendPacket(MessageQueueId_t dest, const store_address_t &storeId) {
  setMsgDestination(dest);
  return sendPacket(storeId);
}

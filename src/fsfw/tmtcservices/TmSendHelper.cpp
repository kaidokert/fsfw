#include "TmSendHelper.h"

#include "fsfw/ipc/MessageQueueSenderIF.h"

TmSendHelper::TmSendHelper() = default;

TmSendHelper::TmSendHelper(MessageQueueIF &queue, InternalErrorReporterIF &reporter,
                           MessageQueueId_t tmtcMsgDest)
    : defaultDest(tmtcMsgDest), queue(&queue), errReporter(&reporter) {}

TmSendHelper::TmSendHelper(MessageQueueIF &queue, InternalErrorReporterIF &reporter)
    : queue(&queue), errReporter(&reporter) {}

TmSendHelper::TmSendHelper(InternalErrorReporterIF &reporter) : errReporter(&reporter) {}

ReturnValue_t TmSendHelper::sendPacket(const store_address_t &storeId) {
  if (queue == nullptr) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  TmTcMessage message(storeId);
  ReturnValue_t result = queue->sendMessage(defaultDest, &message, ignoreFault);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    if (errReporter != nullptr and not ignoreFault) {
      errReporter->lostTm();
    }
    return result;
  }
  return result;
}

void TmSendHelper::setDefaultDestination(MessageQueueId_t msgDest) { defaultDest = msgDest; }

void TmSendHelper::setInternalErrorReporter(InternalErrorReporterIF *reporter) {
  errReporter = reporter;
}
void TmSendHelper::setMsgQueue(MessageQueueIF &queue_) { queue = &queue_; }

ReturnValue_t TmSendHelper::sendPacket(MessageQueueId_t dest, const store_address_t &storeId) {
  setDefaultDestination(dest);
  return sendPacket(storeId);
}

MessageQueueId_t TmSendHelper::getDefaultDestination() const { return defaultDest; }

bool TmSendHelper::areFaultsIgnored() const { return ignoreFault; }
void TmSendHelper::ignoreFaults() { ignoreFault = true; }
void TmSendHelper::dontIgnoreFaults() { ignoreFault = false; }

InternalErrorReporterIF *TmSendHelper::getInternalErrorReporter() const { return errReporter; }

MessageQueueIF *TmSendHelper::getMsgQueue() const { return queue; }

#include "TmSendHelper.h"

#include "fsfw/ipc/MessageQueueSenderIF.h"

TmSendHelper::TmSendHelper() = default;

TmSendHelper::TmSendHelper(MessageQueueIF &queue, InternalErrorReporterIF &reporter,
                           MessageQueueId_t defaultDest)
    : queue(&queue), errReporter(&reporter) {
  queue.setDefaultDestination(defaultDest);
}

TmSendHelper::TmSendHelper(MessageQueueIF &queue, InternalErrorReporterIF &reporter)
    : queue(&queue), errReporter(&reporter) {}

TmSendHelper::TmSendHelper(InternalErrorReporterIF &reporter) : errReporter(&reporter) {}

ReturnValue_t TmSendHelper::sendPacket(const store_address_t &storeId) {
  return sendPacket(getDefaultDestination(), storeId);
}

void TmSendHelper::setDefaultDestination(MessageQueueId_t msgDest) {
  if (queue != nullptr) {
    queue->setDefaultDestination(msgDest);
  };
}

void TmSendHelper::setInternalErrorReporter(InternalErrorReporterIF &reporter) {
  errReporter = &reporter;
}
void TmSendHelper::setMsgQueue(MessageQueueIF &queue_) { queue = &queue_; }

ReturnValue_t TmSendHelper::sendPacket(MessageQueueId_t dest, const store_address_t &storeId) {
  if (queue == nullptr) {
    return returnvalue::FAILED;
  }
  TmTcMessage message(storeId);
  ReturnValue_t result = queue->sendMessage(dest, &message, ignoreFault);
  if (result != returnvalue::OK) {
    if (errReporter != nullptr and not ignoreFault) {
      errReporter->lostTm();
    }
    return result;
  }
  return result;
}

MessageQueueId_t TmSendHelper::getDefaultDestination() const {
  if (queue != nullptr) {
    return queue->getDefaultDestination();
  };
  return MessageQueueIF::NO_QUEUE;
}

bool TmSendHelper::areFaultsIgnored() const { return ignoreFault; }
void TmSendHelper::ignoreFaults() { ignoreFault = true; }
void TmSendHelper::dontIgnoreFaults() { ignoreFault = false; }

InternalErrorReporterIF *TmSendHelper::getInternalErrorReporter() const { return errReporter; }

MessageQueueIF *TmSendHelper::getMsgQueue() const { return queue; }

#include "AcceptsTmMock.h"

AcceptsTmMock::AcceptsTmMock(object_id_t registeredId, MessageQueueId_t queueToReturn)
    : SystemObject(registeredId), returnedQueue(queueToReturn) {}

AcceptsTmMock::AcceptsTmMock(MessageQueueId_t queueToReturn)
    : SystemObject(objects::NO_OBJECT, false), returnedQueue(queueToReturn) {}

MessageQueueId_t AcceptsTmMock::getReportReceptionQueue(uint8_t virtualChannel) {
  return returnedQueue;
}

const char* AcceptsTmMock::getName() const { return "TM Acceptor Mock"; }

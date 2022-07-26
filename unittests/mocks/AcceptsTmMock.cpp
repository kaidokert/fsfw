#include "AcceptsTmMock.h"

AcceptsTmMock::AcceptsTmMock(MessageQueueId_t queueToReturn) : returnedQueue(queueToReturn) {}

MessageQueueId_t AcceptsTmMock::getReportReceptionQueue(uint8_t virtualChannel) {
  return returnedQueue;
}

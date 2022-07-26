#ifndef FSFW_TESTS_ACCEPTSTMMOCK_H
#define FSFW_TESTS_ACCEPTSTMMOCK_H

#include "fsfw/tmtcservices/AcceptsTelemetryIF.h"

class AcceptsTmMock : public AcceptsTelemetryIF {
 public:
  explicit AcceptsTmMock(MessageQueueId_t queueToReturn);

  MessageQueueId_t getReportReceptionQueue(uint8_t virtualChannel) override;

  MessageQueueId_t returnedQueue;
};
#endif  // FSFW_TESTS_ACCEPTSTMMOCK_H

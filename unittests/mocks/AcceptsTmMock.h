#ifndef FSFW_TESTS_ACCEPTSTMMOCK_H
#define FSFW_TESTS_ACCEPTSTMMOCK_H

#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/tmtcservices/AcceptsTelemetryIF.h"

class AcceptsTmMock : public SystemObject, public AcceptsTelemetryIF {
 public:
  AcceptsTmMock(object_id_t registeredId, MessageQueueId_t queueToReturn);
  explicit AcceptsTmMock(MessageQueueId_t queueToReturn);

  MessageQueueId_t getReportReceptionQueue(uint8_t virtualChannel) override;

  MessageQueueId_t returnedQueue;
};
#endif  // FSFW_TESTS_ACCEPTSTMMOCK_H

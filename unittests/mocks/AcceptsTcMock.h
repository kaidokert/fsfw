#ifndef FSFW_TESTS_ACCEPTSTCMOCK_H
#define FSFW_TESTS_ACCEPTSTCMOCK_H

#include "fsfw/tmtcservices/AcceptsTelecommandsIF.h"

class AcceptsTcMock : public AcceptsTelecommandsIF {
 public:
  AcceptsTcMock(const char* name, uint32_t id, MessageQueueId_t queueId);
  [[nodiscard]] const char* getName() const override;
  [[nodiscard]] uint32_t getIdentifier() const override;
  [[nodiscard]] MessageQueueId_t getRequestQueue() const override;

  const char* name;
  uint32_t id;
  MessageQueueId_t queueId;

 private:
};

#endif  // FSFW_TESTS_ACCEPTSTCMOCK_H

#ifndef FSFW_UNITTEST_TESTS_MOCKS_HKRECEIVERMOCK_H_
#define FSFW_UNITTEST_TESTS_MOCKS_HKRECEIVERMOCK_H_

#include <fsfw/housekeeping/AcceptsHkPacketsIF.h>

class HkReceiverMock : public AcceptsHkPacketsIF {
 public:
  explicit HkReceiverMock(MessageQueueId_t queueId) : queueId(queueId) {}

  [[nodiscard]] MessageQueueId_t getHkQueue() const override { return queueId; }

 private:
  MessageQueueId_t queueId;
};

#endif /* FSFW_UNITTEST_TESTS_MOCKS_HKRECEIVERMOCK_H_ */

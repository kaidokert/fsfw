#include "testMq.h"

#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/ipc/QueueFactory.h>

#include <array>

#include "fsfw_tests/internal/UnittDefinitions.h"

void testmq::testMq() {
  std::string id = "[testMq]";
  MessageQueueIF* testSenderMq = QueueFactory::instance()->createMessageQueue(1);
  MessageQueueId_t testSenderMqId = testSenderMq->getId();

  MessageQueueIF* testReceiverMq = QueueFactory::instance()->createMessageQueue(1);
  MessageQueueId_t testReceiverMqId = testReceiverMq->getId();
  std::array<uint8_t, 20> testData{0};
  testData[0] = 42;
  MessageQueueMessage testMessage(testData.data(), 1);
  testSenderMq->setDefaultDestination(testReceiverMqId);

  auto result = testSenderMq->sendMessage(testReceiverMqId, &testMessage);
  if (result != returnvalue::OK) {
    unitt::put_error(id);
  }
  MessageQueueMessage recvMessage;
  result = testReceiverMq->receiveMessage(&recvMessage);
  if (result != returnvalue::OK or recvMessage.getData()[0] != 42) {
    unitt::put_error(id);
  }

  result = testSenderMq->sendMessage(testReceiverMqId, &testMessage);
  if (result != returnvalue::OK) {
    unitt::put_error(id);
  }
  MessageQueueId_t senderId = 0;
  result = testReceiverMq->receiveMessage(&recvMessage, &senderId);
  if (result != returnvalue::OK or recvMessage.getData()[0] != 42) {
    unitt::put_error(id);
  }
  if (senderId != testSenderMqId) {
    unitt::put_error(id);
  }
  senderId = testReceiverMq->getLastPartner();
  if (senderId != testSenderMqId) {
    unitt::put_error(id);
  }
}

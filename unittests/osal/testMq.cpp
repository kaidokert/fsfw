#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/ipc/QueueFactory.h>

#include <array>
#include <catch2/catch_test_macros.hpp>

#include "CatchDefinitions.h"

TEST_CASE("MessageQueue Basic Test", "[TestMq]") {
  MessageQueueIF* testSenderMq = QueueFactory::instance()->createMessageQueue(1);
  MessageQueueId_t testSenderMqId = testSenderMq->getId();

  MessageQueueIF* testReceiverMq = QueueFactory::instance()->createMessageQueue(1);
  MessageQueueId_t testReceiverMqId = testReceiverMq->getId();
  std::array<uint8_t, 20> testData{0};
  testData[0] = 42;
  MessageQueueMessage testMessage(testData.data(), 1);
  testSenderMq->setDefaultDestination(testReceiverMqId);

  SECTION("Simple Tests") {
    auto result = testSenderMq->sendMessage(testReceiverMqId, &testMessage);
    REQUIRE(result == returnvalue::OK);
    MessageQueueMessage recvMessage;
    result = testReceiverMq->receiveMessage(&recvMessage);
    REQUIRE(result == returnvalue::OK);
    CHECK(recvMessage.getData()[0] == 42);

    result = testSenderMq->sendMessage(testReceiverMqId, &testMessage);
    REQUIRE(result == returnvalue::OK);
    MessageQueueId_t senderId = 0;
    result = testReceiverMq->receiveMessage(&recvMessage, &senderId);
    REQUIRE(result == returnvalue::OK);
    CHECK(recvMessage.getData()[0] == 42);
    CHECK(senderId == testSenderMqId);
    senderId = testReceiverMq->getLastPartner();
    CHECK(senderId == testSenderMqId);
  }
  SECTION("Test Full") {
    auto result = testSenderMq->sendMessage(testReceiverMqId, &testMessage);
    REQUIRE(result == returnvalue::OK);
    result = testSenderMq->sendMessage(testReceiverMqId, &testMessage);
    REQUIRE(result == MessageQueueIF::FULL);
    // We try another message
    result = testSenderMq->sendMessage(testReceiverMqId, &testMessage);
    REQUIRE(result == MessageQueueIF::FULL);
    MessageQueueMessage recvMessage;
    result = testReceiverMq->receiveMessage(&recvMessage);
    REQUIRE(result == returnvalue::OK);
    CHECK(recvMessage.getData()[0] == 42);
    result = testSenderMq->sendMessage(testReceiverMqId, &testMessage);
    REQUIRE(result == returnvalue::OK);
    result = testReceiverMq->receiveMessage(&recvMessage);
    REQUIRE(result == returnvalue::OK);
    CHECK(recvMessage.getData()[0] == 42);
  }
  // We have to clear MQs ourself ATM
  QueueFactory::instance()->deleteMessageQueue(testSenderMq);
  QueueFactory::instance()->deleteMessageQueue(testReceiverMq);
}

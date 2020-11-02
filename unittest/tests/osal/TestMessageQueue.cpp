#include <fsfw/ipc/MessageQueueIF.h>
#include <fsfw/ipc/QueueFactory.h>
#include "catch.hpp"
#include <array>
#include "core/CatchDefinitions.h"

TEST_CASE("MessageQueue Basic Test","[TestMq]") {
	MessageQueueIF* testSenderMq =
			QueueFactory::instance()->createMessageQueue(1);
	MessageQueueId_t testSenderMqId = testSenderMq->getId();

	MessageQueueIF* testReceiverMq =
				QueueFactory::instance()->createMessageQueue(1);
	MessageQueueId_t testReceiverMqId = testReceiverMq->getId();
	std::array<uint8_t, 20> testData { 0 };
	testData[0] = 42;
	MessageQueueMessage testMessage(testData.data(), 1);
	testSenderMq->setDefaultDestination(testReceiverMqId);

	SECTION("Simple Tests") {
		auto result = testSenderMq->sendMessage(testReceiverMqId, &testMessage);
		REQUIRE(result == retval::CATCH_OK);
		MessageQueueMessage recvMessage;
		result = testReceiverMq->receiveMessage(&recvMessage);
		REQUIRE(result == retval::CATCH_OK);
		CHECK(recvMessage.getData()[0] == 42);

		result = testSenderMq->sendMessage(testReceiverMqId, &testMessage);
		REQUIRE(result == retval::CATCH_OK);
		MessageQueueId_t senderId = 0;
		result = testReceiverMq->receiveMessage(&recvMessage,&senderId);
		REQUIRE(result == retval::CATCH_OK);
		CHECK(recvMessage.getData()[0] == 42);
		CHECK(senderId == testSenderMqId);
		senderId = testReceiverMq->getLastPartner();
		CHECK(senderId == testSenderMqId);
	}


}

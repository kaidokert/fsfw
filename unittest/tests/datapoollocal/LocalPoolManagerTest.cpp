#include "LocalPoolOwnerBase.h"

#include <catch2/catch_test_macros.hpp>
#include <fsfw/datapoollocal/HasLocalDataPoolIF.h>
#include <fsfw/datapoollocal/StaticLocalDataSet.h>
#include <fsfw/ipc/CommandMessageCleaner.h>
#include <unittest/core/CatchDefinitions.h>


TEST_CASE("LocalPoolManagerTest" , "[LocManTest]") {
	LocalPoolOwnerBase* poolOwner = objectManager->
			get<LocalPoolOwnerBase>(objects::TEST_LOCAL_POOL_OWNER_BASE);
	REQUIRE(poolOwner != nullptr);
	REQUIRE(poolOwner->initializeHkManager() == retval::CATCH_OK);
	REQUIRE(poolOwner->initializeHkManagerAfterTaskCreation()
			== retval::CATCH_OK);
	REQUIRE(poolOwner->dataset.assignPointers() == retval::CATCH_OK);
	MessageQueueMockBase* mqMock = poolOwner->getMockQueueHandle();
	REQUIRE(mqMock != nullptr);


	SECTION("BasicTest") {
		// Subscribe for message generation on update.
		REQUIRE(poolOwner->subscribeWrapperSetUpdate() == retval::CATCH_OK);
		// Subscribe for an update message.
		poolOwner->dataset.setChanged(true);
		// Now the update message should be generated.
		REQUIRE(poolOwner->hkManager.performHkOperation() == retval::CATCH_OK);
		REQUIRE(mqMock->wasMessageSent() == true);
		CommandMessage messageSent;
		uint8_t messagesSent = 0;
		REQUIRE(mqMock->receiveMessage(&messageSent) == retval::CATCH_OK);
		CHECK(messageSent.getCommand() == static_cast<int>(
				HousekeepingMessage::UPDATE_NOTIFICATION_SET));

		// Should have been reset.
		CHECK(poolOwner->dataset.hasChanged() == false);
		// Set changed again, result should be the same.
		poolOwner->dataset.setChanged(true);
		REQUIRE(poolOwner->hkManager.performHkOperation() == retval::CATCH_OK);

		REQUIRE(mqMock->wasMessageSent(&messagesSent) == true);
		CHECK(messagesSent == 1);
		REQUIRE(mqMock->receiveMessage(&messageSent) == retval::CATCH_OK);
		CHECK(messageSent.getCommand() == static_cast<int>(
				HousekeepingMessage::UPDATE_NOTIFICATION_SET));

		// now subscribe for set update HK as well.
		REQUIRE(poolOwner->subscribeWrapperSetUpdateHk() == retval::CATCH_OK);
		poolOwner->dataset.setChanged(true);
		REQUIRE(poolOwner->hkManager.performHkOperation() == retval::CATCH_OK);
		REQUIRE(mqMock->wasMessageSent(&messagesSent) == true);
		CHECK(messagesSent == 2);
		// first message sent should be the update notification, considering
		// the internal list is a vector checked in insertion order.
		REQUIRE(mqMock->receiveMessage(&messageSent) == retval::CATCH_OK);
		CHECK(messageSent.getCommand() == static_cast<int>(
				HousekeepingMessage::UPDATE_NOTIFICATION_SET));

		REQUIRE(mqMock->receiveMessage(&messageSent) == retval::CATCH_OK);
		CHECK(messageSent.getCommand() == static_cast<int>(
				HousekeepingMessage::HK_REPORT));
		CommandMessageCleaner::clearCommandMessage(&messageSent);
	}
}


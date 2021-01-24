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
    CommandMessage messageSent;
    uint8_t messagesSent = 0;


    SECTION("BasicTest") {
        /* Subscribe for message generation on update. */
        REQUIRE(poolOwner->subscribeWrapperSetUpdate() == retval::CATCH_OK);
        /* Subscribe for an update message. */
        poolOwner->dataset.setChanged(true);
        /* Now the update message should be generated. */
        REQUIRE(poolOwner->poolManager.performHkOperation() == retval::CATCH_OK);
        REQUIRE(mqMock->wasMessageSent() == true);

        REQUIRE(mqMock->receiveMessage(&messageSent) == retval::CATCH_OK);
        CHECK(messageSent.getCommand() == static_cast<int>(
                HousekeepingMessage::UPDATE_NOTIFICATION_SET));

        /* Should have been reset. */
        CHECK(poolOwner->dataset.hasChanged() == false);
        /* Set changed again, result should be the same. */
        poolOwner->dataset.setChanged(true);
        REQUIRE(poolOwner->poolManager.performHkOperation() == retval::CATCH_OK);

        REQUIRE(mqMock->wasMessageSent(&messagesSent) == true);
        CHECK(messagesSent == 1);
        REQUIRE(mqMock->receiveMessage(&messageSent) == retval::CATCH_OK);
        CHECK(messageSent.getCommand() == static_cast<int>(
                HousekeepingMessage::UPDATE_NOTIFICATION_SET));

        /* Now subscribe for set update HK as well. */
        REQUIRE(poolOwner->subscribeWrapperSetUpdateHk() == retval::CATCH_OK);
        poolOwner->dataset.setChanged(true);
        REQUIRE(poolOwner->poolManager.performHkOperation() == retval::CATCH_OK);
        REQUIRE(mqMock->wasMessageSent(&messagesSent) == true);
        CHECK(messagesSent == 2);
        /* first message sent should be the update notification, considering
        the internal list is a vector checked in insertion order. */
        REQUIRE(mqMock->receiveMessage(&messageSent) == retval::CATCH_OK);
        CHECK(messageSent.getCommand() == static_cast<int>(
                HousekeepingMessage::UPDATE_NOTIFICATION_SET));

        REQUIRE(mqMock->receiveMessage(&messageSent) == retval::CATCH_OK);
        CHECK(messageSent.getCommand() == static_cast<int>(
                HousekeepingMessage::HK_REPORT));
        /* Clear message to avoid memory leak, our mock won't do it for us (yet) */
        CommandMessageCleaner::clearCommandMessage(&messageSent);
        /* we need to reset the subscription list because the pool owner
        is a global object. */
        poolOwner->resetSubscriptionList();
    }

    SECTION("SnapshotUpdateTests") {
        /* we need to reset the subscription list because the pool owner
        is a global object. */
        poolOwner->resetSubscriptionList();

        /* Subscribe for snapshot generation on update. */
        REQUIRE(poolOwner->subscribeWrapperSetUpdateSnapshot() == retval::CATCH_OK);
        poolOwner->dataset.setChanged(true);
        REQUIRE(poolOwner->poolManager.performHkOperation() == retval::CATCH_OK);
        REQUIRE(mqMock->wasMessageSent(&messagesSent) == true);
        CHECK(messagesSent == 1);

    }

    SECTION("AdvancedTests") {

        /* Subscribe for variable update as well */
        REQUIRE(not poolOwner->dataset.hasChanged());
        REQUIRE(poolOwner->subscribeWrapperVariableUpdate(lpool::uint8VarId) ==
                retval::CATCH_OK);
        lp_var_t<uint8_t>* poolVar = dynamic_cast<lp_var_t<uint8_t>*>(
                poolOwner->getPoolObjectHandle(lpool::uint8VarId));
        REQUIRE(poolVar != nullptr);
        poolVar->setChanged(true);
        REQUIRE(poolOwner->poolManager.performHkOperation() == retval::CATCH_OK);

        /* Check update notification was sent. */
        REQUIRE(mqMock->wasMessageSent(&messagesSent) == true);
        CHECK(messagesSent == 1);
        /* Should have been reset. */
        CHECK(poolVar->hasChanged() == false);
        REQUIRE(mqMock->receiveMessage(&messageSent) == retval::CATCH_OK);
        CHECK(messageSent.getCommand() == static_cast<int>(
                HousekeepingMessage::UPDATE_NOTIFICATION_VARIABLE));

        /* Now subscribe for the dataset update (HK and update) again */
        REQUIRE(poolOwner->subscribeWrapperSetUpdate() == retval::CATCH_OK);
        REQUIRE(poolOwner->subscribeWrapperSetUpdateHk() == retval::CATCH_OK);

        poolOwner->dataset.setChanged(true);
        REQUIRE(poolOwner->poolManager.performHkOperation() == retval::CATCH_OK);
        /* Now two messages should be sent. */
        REQUIRE(mqMock->wasMessageSent(&messagesSent) == true);
        CHECK(messagesSent == 2);
        mqMock->clearMessages(true);

        poolOwner->dataset.setChanged(true);
        poolVar->setChanged(true);
        REQUIRE(poolOwner->poolManager.performHkOperation() == retval::CATCH_OK);
        /* Now three messages should be sent. */
        REQUIRE(mqMock->wasMessageSent(&messagesSent) == true);
        CHECK(messagesSent == 3);
        REQUIRE(mqMock->receiveMessage(&messageSent) == retval::CATCH_OK);
        CHECK(messageSent.getCommand() == static_cast<int>(
                HousekeepingMessage::UPDATE_NOTIFICATION_VARIABLE));
        REQUIRE(mqMock->receiveMessage(&messageSent) == retval::CATCH_OK);
        CHECK(messageSent.getCommand() == static_cast<int>(
                HousekeepingMessage::UPDATE_NOTIFICATION_SET));
        REQUIRE(mqMock->receiveMessage(&messageSent) == retval::CATCH_OK);
        CHECK(messageSent.getCommand() == static_cast<int>(
                HousekeepingMessage::HK_REPORT));
        CommandMessageCleaner::clearCommandMessage(&messageSent);
        REQUIRE(mqMock->receiveMessage(&messageSent) ==
                static_cast<int>(MessageQueueIF::EMPTY));
    }
}


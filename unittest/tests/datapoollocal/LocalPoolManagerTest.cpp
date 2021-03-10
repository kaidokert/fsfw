#include "LocalPoolOwnerBase.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <fsfw/datapool/PoolReadGuard.h>
#include <fsfw/datapoollocal/HasLocalDataPoolIF.h>
#include <fsfw/datapoollocal/StaticLocalDataSet.h>
#include <fsfw/housekeeping/HousekeepingSnapshot.h>
#include <fsfw/ipc/CommandMessageCleaner.h>
#include <fsfw/timemanager/CCSDSTime.h>
#include <unittest/core/CatchDefinitions.h>
#include <iostream>


TEST_CASE("LocalPoolManagerTest" , "[LocManTest]") {
    LocalPoolOwnerBase* poolOwner = objectManager->
            get<LocalPoolOwnerBase>(objects::TEST_LOCAL_POOL_OWNER_BASE);
    REQUIRE(poolOwner != nullptr);
    REQUIRE(poolOwner->initializeHkManager() == retval::CATCH_OK);
    REQUIRE(poolOwner->initializeHkManagerAfterTaskCreation()
            == retval::CATCH_OK);
    //REQUIRE(poolOwner->dataset.assignPointers() == retval::CATCH_OK);
    MessageQueueMockBase* mqMock = poolOwner->getMockQueueHandle();
    REQUIRE(mqMock != nullptr);
    CommandMessage messageSent;
    uint8_t messagesSent = 0;


    SECTION("BasicTest") {
        auto owner = poolOwner->poolManager.getOwner();
        REQUIRE(owner != nullptr);
        CHECK(owner->getObjectId() == objects::TEST_LOCAL_POOL_OWNER_BASE);

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

    }

    SECTION("SetSnapshotUpdateTest") {
        /* Set the variables in the set to certain values. These are checked later. */
        {
            PoolReadGuard readHelper(&poolOwner->dataset);
            REQUIRE(readHelper.getReadResult() == retval::CATCH_OK);
            poolOwner->dataset.localPoolVarUint8.value = 5;
            poolOwner->dataset.localPoolVarFloat.value = -12.242;
            poolOwner->dataset.localPoolUint16Vec.value[0] = 2;
            poolOwner->dataset.localPoolUint16Vec.value[1] = 32;
            poolOwner->dataset.localPoolUint16Vec.value[2] = 42932;
        }

        /* Subscribe for snapshot generation on update. */
        REQUIRE(poolOwner->subscribeWrapperSetUpdateSnapshot() == retval::CATCH_OK);
        poolOwner->dataset.setChanged(true);

        /* Store current time, we are going to check the (approximate) time equality later */
        CCSDSTime::CDS_short timeCdsNow;
        timeval now;
        Clock::getClock_timeval(&now);
        CCSDSTime::convertToCcsds(&timeCdsNow, &now);

        /* Trigger generation of snapshot */
        REQUIRE(poolOwner->poolManager.performHkOperation() == retval::CATCH_OK);
        REQUIRE(mqMock->wasMessageSent(&messagesSent) == true);
        CHECK(messagesSent == 1);
        REQUIRE(mqMock->receiveMessage(&messageSent) == retval::CATCH_OK);
        /* Check that snapshot was generated */
        CHECK(messageSent.getCommand() == static_cast<int>(
                HousekeepingMessage::UPDATE_SNAPSHOT_SET));
        /* Now we deserialize the snapshot into a new dataset instance */
        CCSDSTime::CDS_short cdsShort;
        LocalPoolTestDataSet newSet;
        HousekeepingSnapshot snapshot(&cdsShort, &newSet);
        store_address_t storeId;
        HousekeepingMessage::getUpdateSnapshotSetCommand(&messageSent, &storeId);
        ConstAccessorPair accessorPair = tglob::getIpcStoreHandle()->getData(storeId);
        REQUIRE(accessorPair.first == retval::CATCH_OK);
        const uint8_t* readOnlyPtr = accessorPair.second.data();
        size_t sizeToDeserialize = accessorPair.second.size();
        CHECK(newSet.localPoolVarFloat.value == 0);
        CHECK(newSet.localPoolVarUint8 == 0);
        CHECK(newSet.localPoolUint16Vec.value[0] == 0);
        CHECK(newSet.localPoolUint16Vec.value[1] == 0);
        CHECK(newSet.localPoolUint16Vec.value[2] == 0);
        /* Fill the dataset and timestamp */
        REQUIRE(snapshot.deSerialize(&readOnlyPtr, &sizeToDeserialize,
                SerializeIF::Endianness::MACHINE) == retval::CATCH_OK);
        /* Now we check that the snapshot is actually correct */
        CHECK(newSet.localPoolVarFloat.value == Catch::Approx(-12.242));
        CHECK(newSet.localPoolVarUint8 == 5);
        CHECK(newSet.localPoolUint16Vec.value[0] == 2);
        CHECK(newSet.localPoolUint16Vec.value[1] == 32);
        CHECK(newSet.localPoolUint16Vec.value[2] == 42932);

        /* Now we check that both times are equal */
        CHECK(cdsShort.pField == timeCdsNow.pField);
        CHECK(cdsShort.dayLSB == Catch::Approx(timeCdsNow.dayLSB).margin(1));
        CHECK(cdsShort.dayMSB == Catch::Approx(timeCdsNow.dayMSB).margin(1));
        CHECK(cdsShort.msDay_h == Catch::Approx(timeCdsNow.msDay_h).margin(1));
        CHECK(cdsShort.msDay_hh == Catch::Approx(timeCdsNow.msDay_hh).margin(1));
        CHECK(cdsShort.msDay_l == Catch::Approx(timeCdsNow.msDay_l).margin(1));
        CHECK(cdsShort.msDay_ll == Catch::Approx(timeCdsNow.msDay_ll).margin(1));
    }

    SECTION("VariableSnapshotTest") {
        /* Acquire subscription interface */
        ProvidesDataPoolSubscriptionIF* subscriptionIF = poolOwner->getSubscriptionInterface();
        REQUIRE(subscriptionIF != nullptr);

        /* Subscribe for variable snapshot */
        REQUIRE(poolOwner->subscribeWrapperVariableSnapshot(lpool::uint8VarId) == retval::CATCH_OK);
        auto poolVar = dynamic_cast<lp_var_t<uint8_t>*>(
                poolOwner->getPoolObjectHandle(lpool::uint8VarId));
        REQUIRE(poolVar != nullptr);
        poolVar->setChanged(true);
        REQUIRE(poolOwner->poolManager.performHkOperation() == retval::CATCH_OK);

        /* Check update snapshot was sent. */
        REQUIRE(mqMock->wasMessageSent(&messagesSent) == true);
        CHECK(messagesSent == 1);

        /* Should have been reset. */
        CHECK(poolVar->hasChanged() == false);
        REQUIRE(mqMock->receiveMessage(&messageSent) == retval::CATCH_OK);
        CHECK(messageSent.getCommand() == static_cast<int>(
                HousekeepingMessage::UPDATE_SNAPSHOT_VARIABLE));
    }

    SECTION("VariableUpdateTest") {
        /* Acquire subscription interface */
        ProvidesDataPoolSubscriptionIF* subscriptionIF = poolOwner->getSubscriptionInterface();
        REQUIRE(subscriptionIF != nullptr);

        /* Subscribe for variable update */
        REQUIRE(poolOwner->subscribeWrapperVariableUpdate(lpool::uint8VarId) ==
                retval::CATCH_OK);
        lp_var_t<uint8_t>* poolVar = dynamic_cast<lp_var_t<uint8_t>*>(
                poolOwner->getPoolObjectHandle(lpool::uint8VarId));
        REQUIRE(poolVar != nullptr);
        poolVar->setChanged(true);
        REQUIRE(poolVar->hasChanged() == true);
        REQUIRE(poolOwner->poolManager.performHkOperation() == retval::CATCH_OK);

        /* Check update notification was sent. */
        REQUIRE(mqMock->wasMessageSent(&messagesSent) == true);
        CHECK(messagesSent == 1);
        /* Should have been reset. */
        CHECK(poolVar->hasChanged() == false);
        REQUIRE(mqMock->receiveMessage(&messageSent) == retval::CATCH_OK);
        CHECK(messageSent.getCommand() == static_cast<int>(
                HousekeepingMessage::UPDATE_NOTIFICATION_VARIABLE));
        /* Now subscribe for the dataset update (HK and update) again with subscription interface */
        REQUIRE(subscriptionIF->subscribeForSetUpdateMessage(lpool::testSetId,
                objects::NO_OBJECT, objects::HK_RECEIVER_MOCK, false) == retval::CATCH_OK);
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
        REQUIRE(mqMock->receiveMessage(&messageSent) == static_cast<int>(MessageQueueIF::EMPTY));
    }

    SECTION("Periodic HK") {
        /* Now we subcribe for a HK periodic generation. Even when it's difficult to simulate
        the temporal behaviour correctly the HK manager should generate a HK packet
        immediately and the periodic helper depends on HK op function calls anyway instead of
        using the clock, so we could also just call performHkOperation multiple times */
        REQUIRE(poolOwner->subscribePeriodicHk() == retval::CATCH_OK);
        REQUIRE(poolOwner->poolManager.performHkOperation() == retval::CATCH_OK);
        /* Now HK packet should be sent as message. */
        REQUIRE(mqMock->wasMessageSent(&messagesSent) == true);
        CHECK(messagesSent == 1);

        LocalPoolDataSetBase* setHandle = poolOwner->getDataSetHandle(lpool::testSid);
        REQUIRE(setHandle != nullptr);
        CHECK(poolOwner->poolManager.generateHousekeepingPacket(lpool::testSid,
                setHandle, false) == retval::CATCH_OK);
        REQUIRE(mqMock->wasMessageSent(&messagesSent) == true);
        CHECK(messagesSent == 1);

        CHECK(setHandle->getReportingEnabled() == true);
        CommandMessage hkCmd;
        HousekeepingMessage::setToggleReportingCommand(&hkCmd, lpool::testSid, false, false);
        CHECK(poolOwner->poolManager.handleHousekeepingMessage(&hkCmd) == retval::CATCH_OK);
        CHECK(setHandle->getReportingEnabled() == false);
        HousekeepingMessage::setToggleReportingCommand(&hkCmd, lpool::testSid, true, false);
        CHECK(poolOwner->poolManager.handleHousekeepingMessage(&hkCmd) == retval::CATCH_OK);
        CHECK(setHandle->getReportingEnabled() == true);
    }

    /* we need to reset the subscription list because the pool owner
    is a global object. */
    CHECK(poolOwner->reset() == retval::CATCH_OK);
    mqMock->clearMessages(true);
}


#include <fsfw/datapool/PoolReadGuard.h>
#include <fsfw/datapoollocal/HasLocalDataPoolIF.h>
#include <fsfw/datapoollocal/StaticLocalDataSet.h>
#include <fsfw/globalfunctions/timevalOperations.h>
#include <fsfw/housekeeping/HousekeepingSnapshot.h>
#include <fsfw/ipc/CommandMessageCleaner.h>
#include <fsfw/objectmanager/ObjectManager.h>
#include <fsfw/timemanager/CCSDSTime.h>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <iostream>

#include "CatchDefinitions.h"
#include "mocks/HkReceiverMock.h"
#include "mocks/LocalPoolOwnerBase.h"

TEST_CASE("Local Pool Manager Tests", "[LocManTest]") {
  const MessageQueueId_t defaultDestId = 1;
  const MessageQueueId_t hkDest = defaultDestId;
  const MessageQueueId_t subscriberId = 2;
  auto hkReceiver = HkReceiverMock(hkDest);
  auto queue = MessageQueueMock(3);
  LocalPoolOwnerBase poolOwner(queue, objects::TEST_LOCAL_POOL_OWNER_BASE);
  REQUIRE(poolOwner.initializeHkManager() == returnvalue::OK);
  REQUIRE(poolOwner.initializeHkManagerAfterTaskCreation() == returnvalue::OK);

  MessageQueueMock& poolOwnerMock = poolOwner.getMockQueueHandle();

  // TODO: This is ugly. This should be an arbitrary ctor argument. Fix this in the pool
  //       manager
  poolOwnerMock.setDefaultDestination(defaultDestId);
  poolOwner.setHkDestId(hkDest);

  auto* hkMan = poolOwner.getHkManagerHandle();

  CommandMessage messageSent;

  SECTION("Basic Test") {
    {
      /* For code coverage, should not crash */
      LocalDataPoolManager manager(nullptr, nullptr);
    }
    auto owner = poolOwner.poolManager.getOwner();
    REQUIRE(owner != nullptr);
    CHECK(owner->getObjectId() == objects::TEST_LOCAL_POOL_OWNER_BASE);

    /* Subscribe for message generation on update. */
    REQUIRE(poolOwner.subscribeWrapperSetUpdate(subscriberId) == returnvalue::OK);
    /* Subscribe for an update message. */
    poolOwner.dataset.setChanged(true);
    /* Now the update message should be generated. */
    REQUIRE(poolOwner.poolManager.performHkOperation() == returnvalue::OK);
    REQUIRE(poolOwnerMock.wasMessageSent());

    REQUIRE(poolOwnerMock.getNextSentMessage(subscriberId, messageSent) == returnvalue::OK);
    CHECK(messageSent.getCommand() ==
          static_cast<int>(HousekeepingMessage::UPDATE_NOTIFICATION_SET));

    /* Should have been reset. */
    CHECK(poolOwner.dataset.hasChanged() == false);
    poolOwnerMock.clearMessages(true);
    /* Set changed again, result should be the same. */
    poolOwner.dataset.setChanged(true);
    REQUIRE(poolOwner.poolManager.performHkOperation() == returnvalue::OK);

    REQUIRE(poolOwnerMock.wasMessageSent() == true);
    CHECK(poolOwnerMock.numberOfSentMessages() == 1);
    REQUIRE(poolOwnerMock.getNextSentMessage(subscriberId, messageSent) == returnvalue::OK);
    CHECK(messageSent.getCommand() ==
          static_cast<int>(HousekeepingMessage::UPDATE_NOTIFICATION_SET));

    poolOwnerMock.clearMessages(true);
    /* Now subscribe for set update HK as well. */
    REQUIRE(poolOwner.subscribeWrapperSetUpdateHk(false, &hkReceiver) == returnvalue::OK);
    poolOwner.dataset.setChanged(true);
    REQUIRE(poolOwner.poolManager.performHkOperation() == returnvalue::OK);
    REQUIRE(poolOwnerMock.wasMessageSent() == true);
    CHECK(poolOwnerMock.numberOfSentMessages() == 2);
    // first message sent should be the update notification
    REQUIRE(poolOwnerMock.getNextSentMessage(subscriberId, messageSent) == returnvalue::OK);
    CHECK(messageSent.getCommand() ==
          static_cast<int>(HousekeepingMessage::UPDATE_NOTIFICATION_SET));
    REQUIRE(poolOwnerMock.getNextSentMessage(messageSent) == returnvalue::OK);
    CHECK(messageSent.getCommand() == static_cast<int>(HousekeepingMessage::HK_REPORT));
    /* Clear message to avoid memory leak, our mock won't do it for us (yet) */
    CommandMessageCleaner::clearCommandMessage(&messageSent);
  }

  SECTION("SetSnapshotUpdateTest") {
    /* Set the variables in the set to certain values. These are checked later. */
    {
      PoolReadGuard readHelper(&poolOwner.dataset);
      REQUIRE(readHelper.getReadResult() == returnvalue::OK);
      poolOwner.dataset.localPoolVarUint8.value = 5;
      poolOwner.dataset.localPoolVarFloat.value = -12.242;
      poolOwner.dataset.localPoolUint16Vec.value[0] = 2;
      poolOwner.dataset.localPoolUint16Vec.value[1] = 32;
      poolOwner.dataset.localPoolUint16Vec.value[2] = 42932;
    }

    /* Subscribe for snapshot generation on update. */
    REQUIRE(poolOwner.subscribeWrapperSetUpdateSnapshot(subscriberId) == returnvalue::OK);
    poolOwner.dataset.setChanged(true);

    /* Store current time, we are going to check the (approximate) time equality later */
    timeval now{};
    Clock::getClock_timeval(&now);

    /* Trigger generation of snapshot */
    REQUIRE(poolOwner.poolManager.performHkOperation() == returnvalue::OK);
    REQUIRE(poolOwnerMock.wasMessageSent());
    CHECK(poolOwnerMock.numberOfSentMessages() == 1);
    REQUIRE(poolOwnerMock.getNextSentMessage(subscriberId, messageSent) == returnvalue::OK);
    /* Check that snapshot was generated */
    CHECK(messageSent.getCommand() == static_cast<int>(HousekeepingMessage::UPDATE_SNAPSHOT_SET));
    /* Now we deserialize the snapshot into a new dataset instance */
    CCSDSTime::CDS_short cdsShort{};
    LocalPoolTestDataSet newSet;
    HousekeepingSnapshot snapshot(&cdsShort, &newSet);
    store_address_t storeId;
    HousekeepingMessage::getUpdateSnapshotSetCommand(&messageSent, &storeId);
    ConstAccessorPair accessorPair = tglob::getIpcStoreHandle()->getData(storeId);
    REQUIRE(accessorPair.first == returnvalue::OK);
    const uint8_t* readOnlyPtr = accessorPair.second.data();
    size_t sizeToDeserialize = accessorPair.second.size();
    CHECK(newSet.localPoolVarFloat.value == 0);
    CHECK(newSet.localPoolVarUint8 == 0);
    CHECK(newSet.localPoolUint16Vec.value[0] == 0);
    CHECK(newSet.localPoolUint16Vec.value[1] == 0);
    CHECK(newSet.localPoolUint16Vec.value[2] == 0);
    /* Fill the dataset and timestamp */
    REQUIRE(snapshot.deSerialize(&readOnlyPtr, &sizeToDeserialize,
                                 SerializeIF::Endianness::MACHINE) == returnvalue::OK);
    /* Now we check that the snapshot is actually correct */
    CHECK(newSet.localPoolVarFloat.value == Catch::Approx(-12.242));
    CHECK(newSet.localPoolVarUint8 == 5);
    CHECK(newSet.localPoolUint16Vec.value[0] == 2);
    CHECK(newSet.localPoolUint16Vec.value[1] == 32);
    CHECK(newSet.localPoolUint16Vec.value[2] == 42932);

    /* Now we check that both times are equal */
    timeval timeFromHK{};
    auto result = CCSDSTime::convertFromCDS(&timeFromHK, &cdsShort);
    CHECK(result == returnvalue::OK);
    timeval difference = timeFromHK - now;
    CHECK(timevalOperations::toDouble(difference) < 1.0);
  }

  SECTION("VariableSnapshotTest") {
    /* Acquire subscription interface */
    ProvidesDataPoolSubscriptionIF* subscriptionIF = poolOwner.getSubscriptionInterface();
    REQUIRE(subscriptionIF != nullptr);

    /* Subscribe for variable snapshot */
    REQUIRE(poolOwner.subscribeWrapperVariableSnapshot(subscriberId, lpool::uint8VarId) ==
            returnvalue::OK);
    auto poolVar =
        dynamic_cast<lp_var_t<uint8_t>*>(poolOwner.getPoolObjectHandle(lpool::uint8VarId));
    REQUIRE(poolVar != nullptr);

    {
      PoolReadGuard rg(poolVar);
      CHECK(rg.getReadResult() == returnvalue::OK);
      poolVar->value = 25;
    }

    poolVar->setChanged(true);
    /* Store current time, we are going to check the (approximate) time equality later */
    CCSDSTime::CDS_short timeCdsNow{};
    timeval now{};
    Clock::getClock_timeval(&now);
    REQUIRE(poolOwner.poolManager.performHkOperation() == returnvalue::OK);

    /* Check update snapshot was sent. */
    REQUIRE(poolOwnerMock.wasMessageSent());
    CHECK(poolOwnerMock.numberOfSentMessages() == 1);

    /* Should have been reset. */
    CHECK(poolVar->hasChanged() == false);
    REQUIRE(poolOwnerMock.getNextSentMessage(subscriberId, messageSent) == returnvalue::OK);
    CHECK(messageSent.getCommand() ==
          static_cast<int>(HousekeepingMessage::UPDATE_SNAPSHOT_VARIABLE));
    /* Now we deserialize the snapshot into a new dataset instance */
    CCSDSTime::CDS_short cdsShort{};
    lp_var_t<uint8_t> varCopy = lp_var_t<uint8_t>(lpool::uint8VarGpid);
    HousekeepingSnapshot snapshot(&cdsShort, &varCopy);
    store_address_t storeId;
    HousekeepingMessage::getUpdateSnapshotVariableCommand(&messageSent, &storeId);
    ConstAccessorPair accessorPair = tglob::getIpcStoreHandle()->getData(storeId);
    REQUIRE(accessorPair.first == returnvalue::OK);
    const uint8_t* readOnlyPtr = accessorPair.second.data();
    size_t sizeToDeserialize = accessorPair.second.size();
    CHECK(varCopy.value == 0);
    /* Fill the dataset and timestamp */
    REQUIRE(snapshot.deSerialize(&readOnlyPtr, &sizeToDeserialize,
                                 SerializeIF::Endianness::MACHINE) == returnvalue::OK);
    CHECK(varCopy.value == 25);

    /* Now we check that both times are equal */
    timeval timeFromHK{};
    auto result = CCSDSTime::convertFromCDS(&timeFromHK, &cdsShort);
    CHECK(result == returnvalue::OK);
    timeval difference = timeFromHK - now;
    CHECK(timevalOperations::toDouble(difference) < 1.0);
  }

  SECTION("VariableNotificationTest") {
    /* Acquire subscription interface */
    ProvidesDataPoolSubscriptionIF* subscriptionIF = poolOwner.getSubscriptionInterface();
    REQUIRE(subscriptionIF != nullptr);

    /* Subscribe for variable update */
    REQUIRE(poolOwner.subscribeWrapperVariableUpdate(subscriberId, lpool::uint8VarId) ==
            returnvalue::OK);
    auto* poolVar =
        dynamic_cast<lp_var_t<uint8_t>*>(poolOwner.getPoolObjectHandle(lpool::uint8VarId));
    REQUIRE(poolVar != nullptr);
    poolVar->setChanged(true);
    REQUIRE(poolVar->hasChanged() == true);
    REQUIRE(poolOwner.poolManager.performHkOperation() == returnvalue::OK);

    /* Check update notification was sent. */
    REQUIRE(poolOwnerMock.wasMessageSent());
    CHECK(poolOwnerMock.numberOfSentMessages() == 1);
    /* Should have been reset. */
    CHECK(poolVar->hasChanged() == false);
    REQUIRE(poolOwnerMock.getNextSentMessage(subscriberId, messageSent) == returnvalue::OK);
    CHECK(messageSent.getCommand() ==
          static_cast<int>(HousekeepingMessage::UPDATE_NOTIFICATION_VARIABLE));
    /* Now subscribe for the dataset update (HK and update) again with subscription interface */
    REQUIRE(subscriptionIF->subscribeForSetUpdateMessage(lpool::testSetId, objects::NO_OBJECT,
                                                         subscriberId, false) == returnvalue::OK);
    REQUIRE(poolOwner.subscribeWrapperSetUpdateHk(false, &hkReceiver) == returnvalue::OK);

    poolOwner.dataset.setChanged(true);
    poolOwnerMock.clearMessages();
    REQUIRE(poolOwner.poolManager.performHkOperation() == returnvalue::OK);
    /* Now two messages should be sent. */
    REQUIRE(poolOwnerMock.wasMessageSent());
    CHECK(poolOwnerMock.numberOfSentMessages() == 2);
    poolOwnerMock.clearMessages(true);

    poolOwner.dataset.setChanged(true);
    poolOwnerMock.clearMessages(true);
    poolVar->setChanged(true);
    REQUIRE(poolOwner.poolManager.performHkOperation() == returnvalue::OK);
    /* Now three messages should be sent. */
    REQUIRE(poolOwnerMock.wasMessageSent());
    CHECK(poolOwnerMock.numberOfSentMessages() == 3);
    CHECK(poolOwnerMock.numberOfSentMessagesToDest(subscriberId) == 2);
    CHECK(poolOwnerMock.numberOfSentMessagesToDest(hkDest) == 1);
    REQUIRE(poolOwnerMock.getNextSentMessage(subscriberId, messageSent) == returnvalue::OK);
    CHECK(messageSent.getCommand() ==
          static_cast<int>(HousekeepingMessage::UPDATE_NOTIFICATION_VARIABLE));
    REQUIRE(poolOwnerMock.clearLastSentMessage(subscriberId) == returnvalue::OK);
    REQUIRE(poolOwnerMock.getNextSentMessage(subscriberId, messageSent) == returnvalue::OK);
    CHECK(messageSent.getCommand() ==
          static_cast<int>(HousekeepingMessage::UPDATE_NOTIFICATION_SET));
    REQUIRE(poolOwnerMock.clearLastSentMessage(subscriberId) == returnvalue::OK);
    REQUIRE(poolOwnerMock.getNextSentMessage(messageSent) == returnvalue::OK);
    CHECK(messageSent.getCommand() == static_cast<int>(HousekeepingMessage::HK_REPORT));
    REQUIRE(poolOwnerMock.clearLastSentMessage() == returnvalue::OK);
    REQUIRE(poolOwnerMock.getNextSentMessage(subscriberId, messageSent) == MessageQueueIF::EMPTY);
    REQUIRE(poolOwnerMock.getNextSentMessage(messageSent) == MessageQueueIF::EMPTY);
  }

  SECTION("PeriodicHKAndMessaging") {
    /* Now we subcribe for a HK periodic generation. Even when it's difficult to simulate
    the temporal behaviour correctly the HK manager should generate a HK packet
    immediately and the periodic helper depends on HK op function calls anyway instead of
    using the clock, so we could also just call performHkOperation multiple times */
    REQUIRE(poolOwner.subscribePeriodicHk(true) == returnvalue::OK);
    REQUIRE(poolOwner.poolManager.performHkOperation() == returnvalue::OK);
    /* Now HK packet should be sent as message immediately. */
    REQUIRE(poolOwnerMock.wasMessageSent());
    CHECK(poolOwnerMock.numberOfSentMessages() == 1);
    CHECK(poolOwnerMock.clearLastSentMessage() == returnvalue::OK);

    LocalPoolDataSetBase* setHandle = poolOwner.getDataSetHandle(lpool::testSid);
    REQUIRE(setHandle != nullptr);
    CHECK(poolOwner.poolManager.generateHousekeepingPacket(lpool::testSid, setHandle, false) ==
          returnvalue::OK);
    REQUIRE(poolOwnerMock.wasMessageSent());
    CHECK(poolOwnerMock.numberOfSentMessages() == 1);
    CHECK(poolOwnerMock.clearLastSentMessage() == returnvalue::OK);

    CHECK(setHandle->getReportingEnabled() == true);
    CommandMessage hkCmd;
    HousekeepingMessage::setToggleReportingCommand(&hkCmd, lpool::testSid, false, false);
    CHECK(poolOwner.poolManager.handleHousekeepingMessage(&hkCmd) == returnvalue::OK);
    CHECK(setHandle->getReportingEnabled() == false);
    REQUIRE(poolOwnerMock.wasMessageSent());
    CHECK(poolOwnerMock.numberOfSentMessages() == 1);
    CHECK(poolOwnerMock.clearLastSentMessage() == returnvalue::OK);

    HousekeepingMessage::setToggleReportingCommand(&hkCmd, lpool::testSid, true, false);
    CHECK(poolOwner.poolManager.handleHousekeepingMessage(&hkCmd) == returnvalue::OK);
    CHECK(setHandle->getReportingEnabled() == true);
    REQUIRE(poolOwnerMock.wasMessageSent());
    CHECK(poolOwnerMock.clearLastSentMessage() == returnvalue::OK);

    HousekeepingMessage::setToggleReportingCommand(&hkCmd, lpool::testSid, false, false);
    CHECK(poolOwner.poolManager.handleHousekeepingMessage(&hkCmd) == returnvalue::OK);
    CHECK(setHandle->getReportingEnabled() == false);
    REQUIRE(poolOwnerMock.wasMessageSent());
    CHECK(poolOwnerMock.clearLastSentMessage() == returnvalue::OK);

    HousekeepingMessage::setCollectionIntervalModificationCommand(&hkCmd, lpool::testSid, 0.4,
                                                                  false);
    CHECK(poolOwner.poolManager.handleHousekeepingMessage(&hkCmd) == returnvalue::OK);
    /* For non-diagnostics and a specified minimum frequency of 0.2 seconds, the
    resulting collection interval should be 1.0 second */
    CHECK(poolOwner.dataset.getCollectionInterval() == 1.0);
    REQUIRE(poolOwnerMock.wasMessageSent());
    REQUIRE(poolOwnerMock.numberOfSentMessages() == 1);
    CHECK(poolOwnerMock.clearLastSentMessage() == returnvalue::OK);

    HousekeepingMessage::setStructureReportingCommand(&hkCmd, lpool::testSid, false);
    REQUIRE(poolOwner.poolManager.performHkOperation() == returnvalue::OK);
    CHECK(poolOwner.poolManager.handleHousekeepingMessage(&hkCmd) == returnvalue::OK);
    /* Now HK packet should be sent as message. */
    REQUIRE(poolOwnerMock.wasMessageSent());
    REQUIRE(poolOwnerMock.numberOfSentMessages() == 1);
    poolOwnerMock.clearMessages();

    HousekeepingMessage::setOneShotReportCommand(&hkCmd, lpool::testSid, false);
    CHECK(poolOwner.poolManager.handleHousekeepingMessage(&hkCmd) == returnvalue::OK);
    REQUIRE(poolOwnerMock.wasMessageSent());
    REQUIRE(poolOwnerMock.numberOfSentMessages() == 1);
    poolOwnerMock.clearMessages();

    HousekeepingMessage::setUpdateNotificationSetCommand(&hkCmd, lpool::testSid);
    sid_t sidToCheck;
    store_address_t storeId;
    CHECK(poolOwner.poolManager.handleHousekeepingMessage(&hkCmd) == returnvalue::OK);
    CHECK(poolOwner.changedDataSetCallbackWasCalled(sidToCheck, storeId) == true);
    CHECK(sidToCheck == lpool::testSid);

    /* Now we test the handling is the dataset is set to diagnostic */
    poolOwner.dataset.setDiagnostic(true);

    HousekeepingMessage::setStructureReportingCommand(&hkCmd, lpool::testSid, false);
    CHECK(poolOwner.poolManager.handleHousekeepingMessage(&hkCmd) ==
          static_cast<int>(LocalDataPoolManager::WRONG_HK_PACKET_TYPE));
    /* We still expect a failure message being sent */
    REQUIRE(poolOwnerMock.wasMessageSent());
    REQUIRE(poolOwnerMock.numberOfSentMessages() == 1);
    CHECK(poolOwnerMock.clearLastSentMessage() == returnvalue::OK);

    HousekeepingMessage::setCollectionIntervalModificationCommand(&hkCmd, lpool::testSid, 0.4,
                                                                  false);
    CHECK(poolOwner.poolManager.handleHousekeepingMessage(&hkCmd) ==
          static_cast<int>(LocalDataPoolManager::WRONG_HK_PACKET_TYPE));
    REQUIRE(poolOwnerMock.wasMessageSent());
    REQUIRE(poolOwnerMock.numberOfSentMessages() == 1);
    CHECK(poolOwnerMock.clearLastSentMessage() == returnvalue::OK);

    HousekeepingMessage::setStructureReportingCommand(&hkCmd, lpool::testSid, false);
    CHECK(poolOwner.poolManager.handleHousekeepingMessage(&hkCmd) ==
          static_cast<int>(LocalDataPoolManager::WRONG_HK_PACKET_TYPE));
    REQUIRE(poolOwnerMock.wasMessageSent());
    REQUIRE(poolOwnerMock.numberOfSentMessages() == 1);
    CHECK(poolOwnerMock.clearLastSentMessage() == returnvalue::OK);

    HousekeepingMessage::setStructureReportingCommand(&hkCmd, lpool::testSid, true);
    CHECK(poolOwner.poolManager.handleHousekeepingMessage(&hkCmd) == returnvalue::OK);
    REQUIRE(poolOwnerMock.wasMessageSent());
    REQUIRE(poolOwnerMock.numberOfSentMessages() == 1);
    poolOwnerMock.clearMessages();

    HousekeepingMessage::setCollectionIntervalModificationCommand(&hkCmd, lpool::testSid, 0.4,
                                                                  true);
    CHECK(poolOwner.poolManager.handleHousekeepingMessage(&hkCmd) == returnvalue::OK);
    REQUIRE(poolOwnerMock.wasMessageSent());
    REQUIRE(poolOwnerMock.numberOfSentMessages() == 1);
    poolOwnerMock.clearMessages();

    HousekeepingMessage::setToggleReportingCommand(&hkCmd, lpool::testSid, true, true);
    CHECK(poolOwner.poolManager.handleHousekeepingMessage(&hkCmd) == returnvalue::OK);
    REQUIRE(poolOwnerMock.wasMessageSent());
    REQUIRE(poolOwnerMock.numberOfSentMessages() == 1);
    poolOwnerMock.clearMessages();

    HousekeepingMessage::setToggleReportingCommand(&hkCmd, lpool::testSid, false, true);
    CHECK(poolOwner.poolManager.handleHousekeepingMessage(&hkCmd) == returnvalue::OK);
    REQUIRE(poolOwnerMock.wasMessageSent());
    REQUIRE(poolOwnerMock.numberOfSentMessages() == 1);
    poolOwnerMock.clearMessages();

    HousekeepingMessage::setOneShotReportCommand(&hkCmd, lpool::testSid, false);
    CHECK(poolOwner.poolManager.handleHousekeepingMessage(&hkCmd) ==
          static_cast<int>(LocalDataPoolManager::WRONG_HK_PACKET_TYPE));
    REQUIRE(poolOwnerMock.wasMessageSent());
    REQUIRE(poolOwnerMock.numberOfSentMessages() == 1);
    poolOwnerMock.clearMessages();

    HousekeepingMessage::setOneShotReportCommand(&hkCmd, lpool::testSid, true);
    CHECK(poolOwner.poolManager.handleHousekeepingMessage(&hkCmd) == returnvalue::OK);
    REQUIRE(poolOwnerMock.wasMessageSent());
    REQUIRE(poolOwnerMock.numberOfSentMessages() == 1);
    poolOwnerMock.clearMessages();

    HousekeepingMessage::setUpdateNotificationVariableCommand(&hkCmd, lpool::uint8VarGpid);
    gp_id_t gpidToCheck;
    CHECK(poolOwner.poolManager.handleHousekeepingMessage(&hkCmd) == returnvalue::OK);
    CHECK(poolOwner.changedVariableCallbackWasCalled(gpidToCheck, storeId) == true);
    CHECK(gpidToCheck == lpool::uint8VarGpid);

    HousekeepingMessage::setUpdateSnapshotSetCommand(&hkCmd, lpool::testSid,
                                                     store_address_t::invalid());
    CHECK(poolOwner.poolManager.handleHousekeepingMessage(&hkCmd) == returnvalue::OK);
    CHECK(poolOwner.changedDataSetCallbackWasCalled(sidToCheck, storeId) == true);
    CHECK(sidToCheck == lpool::testSid);

    HousekeepingMessage::setUpdateSnapshotVariableCommand(&hkCmd, lpool::uint8VarGpid,
                                                          store_address_t::invalid());
    CHECK(poolOwner.poolManager.handleHousekeepingMessage(&hkCmd) == returnvalue::OK);
    CHECK(poolOwner.changedVariableCallbackWasCalled(gpidToCheck, storeId) == true);
    CHECK(gpidToCheck == lpool::uint8VarGpid);

    poolOwner.poolManager.printPoolEntry(lpool::uint8VarId);
  }

  /* we need to reset the subscription list because the pool owner
  is a global object. */
  CHECK(poolOwner.reset() == returnvalue::OK);
  poolOwnerMock.clearMessages(true);
}

#include <catch2/catch_test_macros.hpp>

#include "fsfw/storagemanager/LocalPool.h"
#include "fsfw/tmtcservices/TmSendHelper.h"
#include "fsfw/tmtcservices/TmStoreHelper.h"
#include "mocks/CdsShortTimestamperMock.h"
#include "mocks/InternalErrorReporterMock.h"
#include "mocks/MessageQueueMock.h"

TEST_CASE("TM Send Helper", "[tm-send-helper]") {
  MessageQueueId_t destId = 2;
  auto errReporter = InternalErrorReporterMock();
  auto msgQueue = MessageQueueMock(1);
  msgQueue.setDefaultDestination(destId);
  TmSendHelper sendHelper(msgQueue, errReporter, destId);
  auto timeStamper = CdsShortTimestamperMock();
  LocalPool::LocalPoolConfig cfg = {{5, 32}, {2, 64}};
  LocalPool pool(objects::NO_OBJECT, cfg);
  auto storeHelper = TmStoreHelper(2, pool, timeStamper);

  SECTION("State and Setters") {
    REQUIRE(sendHelper.getInternalErrorReporter() == &errReporter);
    REQUIRE(sendHelper.getDefaultDestination() == destId);
    REQUIRE(sendHelper.getMsgQueue() == &msgQueue);
  }

  SECTION("Setters") {
    REQUIRE(not sendHelper.areFaultsIgnored());
    sendHelper.ignoreFaults();
    REQUIRE(sendHelper.areFaultsIgnored());
    sendHelper.dontIgnoreFaults();
    REQUIRE(not sendHelper.areFaultsIgnored());
    REQUIRE(sendHelper.getDefaultDestination() == destId);
    sendHelper.setDefaultDestination(destId + 1);
    REQUIRE(sendHelper.getDefaultDestination() == destId + 1);
    auto errReporter2 = InternalErrorReporterMock();
    sendHelper.setInternalErrorReporter(errReporter2);
    REQUIRE(sendHelper.getInternalErrorReporter() == &errReporter2);
  }

  SECTION("Default CTOR") {
    TmSendHelper emptyHelper;
    REQUIRE(emptyHelper.getInternalErrorReporter() == nullptr);
    REQUIRE(emptyHelper.getDefaultDestination() == MessageQueueIF::NO_QUEUE);
    store_address_t dummy;
    // basic robustness
    REQUIRE(emptyHelper.sendPacket(dummy) == returnvalue::FAILED);
  }

  SECTION("One Arg CTOR") {
    TmSendHelper helper(errReporter);
    REQUIRE(helper.getInternalErrorReporter() == &errReporter);
    REQUIRE(helper.getDefaultDestination() == MessageQueueIF::NO_QUEUE);
    REQUIRE(helper.getMsgQueue() == nullptr);
  }

  SECTION("Two Arg CTOR") {
    TmSendHelper helper(msgQueue, errReporter);
    REQUIRE(helper.getInternalErrorReporter() == &errReporter);
    REQUIRE(helper.getDefaultDestination() == 2);
    REQUIRE(helper.getMsgQueue() == &msgQueue);
  }
  SECTION("Send") {
    REQUIRE(storeHelper.preparePacket(17, 2, 0) == returnvalue::OK);
    store_address_t storeId;
    SECTION("Separate Helpers") {
      REQUIRE(storeHelper.addPacketToStore() == returnvalue::OK);
      storeId = storeHelper.getCurrentAddr();
      REQUIRE(sendHelper.sendPacket(storeId) == returnvalue::OK);
    }
    REQUIRE(msgQueue.wasMessageSent());
    REQUIRE(msgQueue.numberOfSentMessagesToDefault() == 1);
    TmTcMessage msg;
    REQUIRE(msgQueue.getNextSentMessage(msg) == returnvalue::OK);
    REQUIRE(msg.getStorageId() == storeId);
    REQUIRE(pool.hasDataAtId(msg.getStorageId()));
  }

  SECTION("Send to Non-Default") {
    storeHelper.preparePacket(17, 2, 0);
    REQUIRE(storeHelper.addPacketToStore() == returnvalue::OK);
    store_address_t storeId = storeHelper.getCurrentAddr();
    REQUIRE(sendHelper.sendPacket(destId + 1, storeId) == returnvalue::OK);
    REQUIRE(msgQueue.wasMessageSent());
    REQUIRE(msgQueue.numberOfSentMessagesToDest(destId + 1) == 1);
  }

  SECTION("Sending fails, errors not ignored") {
    msgQueue.makeNextSendFail(returnvalue::FAILED);
    storeHelper.preparePacket(17, 2, 0);
    REQUIRE(storeHelper.addPacketToStore() == returnvalue::OK);
    store_address_t storeId = storeHelper.getCurrentAddr();
    REQUIRE(sendHelper.sendPacket(destId + 1, storeId) == returnvalue::FAILED);
    REQUIRE(errReporter.lostTmCallCnt == 1);
  }

  SECTION("Sending fails, errors ignored") {
    msgQueue.makeNextSendFail(returnvalue::FAILED);
    storeHelper.preparePacket(17, 2, 0);
    sendHelper.ignoreFaults();
    REQUIRE(storeHelper.addPacketToStore() == returnvalue::OK);
    store_address_t storeId = storeHelper.getCurrentAddr();
    REQUIRE(sendHelper.sendPacket(destId + 1, storeId) == returnvalue::FAILED);
    REQUIRE(errReporter.lostTmCallCnt == 0);
  }
}
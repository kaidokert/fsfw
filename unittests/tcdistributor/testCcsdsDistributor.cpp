#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/storagemanager/LocalPool.h"
#include "fsfw/tcdistribution/CcsdsDistributor.h"
#include "fsfw/tmtcpacket/ccsds/SpacePacketCreator.h"
#include "mocks/AcceptsTcMock.h"
#include "mocks/CcsdsCheckerMock.h"
#include "mocks/MessageQueueMock.h"

TEST_CASE("CCSDS Distributor", "[ccsds-distrib]") {
  LocalPool::LocalPoolConfig cfg = {{5, 32}, {2, 64}};
  LocalPool pool(objects::NO_OBJECT, cfg);
  auto queue = MessageQueueMock(1);
  auto checkerMock = CcsdsCheckerMock();
  uint16_t unregisteredApid = 0;
  uint16_t defaultApid = 4;
  MessageQueueId_t defaultQueueId = 5;
  auto ccsdsDistrib = CcsdsDistributor(defaultApid, 1, &pool, &queue, &checkerMock);
  uint32_t tcAcceptorApid = 1;
  MessageQueueId_t tcAcceptorQueueId = 3;

  auto tcAcceptorMock = AcceptsTcMock("TC Receiver Dummy", tcAcceptorApid, tcAcceptorQueueId);
  auto defReceiverMock = AcceptsTcMock("Default Receiver Dummy", defaultApid, defaultQueueId);
  auto packetId = PacketId(ccsds::PacketType::TC, true, 0);
  auto psc = PacketSeqCtrl(ccsds::SequenceFlags::FIRST_SEGMENT, 0x34);
  auto spParams = SpacePacketParams(packetId, psc, 0x16);
  SpacePacketCreator spCreator(spParams);
  std::array<uint8_t, 32> buf{};

  auto createSpacePacket = [&](uint16_t apid, TmTcMessage& msg) {
    store_address_t storeId{};
    spCreator.setApid(tcAcceptorApid);
    uint8_t* dataPtr;
    REQUIRE(pool.getFreeElement(&storeId, spCreator.getSerializedSize(), &dataPtr) == result::OK);
    size_t serLen = 0;
    REQUIRE(spCreator.SerializeIF::serializeBe(dataPtr, serLen, ccsds::HEADER_LEN) == result::OK);
    REQUIRE(spCreator.SerializeIF::serializeBe(buf.data(), serLen, ccsds::HEADER_LEN) ==
            result::OK);
    msg.setStorageId(storeId);
  };

  SECTION("State") {
    CHECK(ccsdsDistrib.initialize() == result::OK);
    CHECK(ccsdsDistrib.getRequestQueue() == 1);
    CHECK(ccsdsDistrib.getIdentifier() == 0);
    CHECK(ccsdsDistrib.getObjectId() == 1);
    REQUIRE(ccsdsDistrib.getName() != nullptr);
    CHECK(std::strcmp(ccsdsDistrib.getName(), "CCSDS Distributor") == 0);
  }

  SECTION("Basic Forwarding") {
    CcsdsDistributor::DestInfo info(tcAcceptorMock, false);
    REQUIRE(ccsdsDistrib.registerApplication(info) == result::OK);
    TmTcMessage message;
    createSpacePacket(tcAcceptorApid, message);
    store_address_t storeId = message.getStorageId();
    queue.addReceivedMessage(message);
    REQUIRE(ccsdsDistrib.performOperation(0) == result::OK);
    CHECK(checkerMock.checkedPacketLen == 6);
    CHECK(checkerMock.checkCallCount == 1);
    CHECK(queue.wasMessageSent());
    CHECK(queue.numberOfSentMessages() == 1);
    // The packet is forwarded, with no need to delete the data
    CHECK(pool.hasDataAtId(storeId));
    TmTcMessage sentMsg;
    CHECK(queue.getNextSentMessage(tcAcceptorQueueId, sentMsg) == result::OK);
    CHECK(sentMsg.getStorageId() == storeId);
    auto accessor = pool.getData(storeId);
    CHECK(accessor.first == result::OK);
    CHECK(accessor.second.size() == ccsds::HEADER_LEN);
    for (size_t i = 0; i < ccsds::HEADER_LEN; i++) {
      CHECK(accessor.second.data()[i] == buf[i]);
    }
  }

  SECTION("Forwarding to Default Destination, but not registered") {
    TmTcMessage message;
    createSpacePacket(unregisteredApid, message);
    store_address_t storeId = message.getStorageId();
    message.setStorageId(storeId);
    queue.addReceivedMessage(message);
    REQUIRE(ccsdsDistrib.performOperation(0) == TcDistributorBase::DESTINATION_NOT_FOUND);
  }

  SECTION("Forward to Default Handler") {
    CcsdsDistributor::DestInfo info(defReceiverMock, false);
    ccsdsDistrib.registerApplication(info);
    TmTcMessage message;
    createSpacePacket(defaultApid, message);
    store_address_t storeId = message.getStorageId();
    message.setStorageId(storeId);
    queue.addReceivedMessage(message);
    REQUIRE(ccsdsDistrib.performOperation(0) == result::OK);
    CHECK(checkerMock.checkedPacketLen == 6);
    CHECK(checkerMock.checkCallCount == 1);
    CHECK(queue.wasMessageSent());
    CHECK(queue.numberOfSentMessages() == 1);
    // The packet is forwarded, with no need to delete the data
    CHECK(pool.hasDataAtId(storeId));
    TmTcMessage sentMsg;
    CHECK(queue.getNextSentMessage(defaultQueueId, sentMsg) == result::OK);
    CHECK(sentMsg.getStorageId() == storeId);
    auto accessor = pool.getData(storeId);
    CHECK(accessor.first == result::OK);
    CHECK(accessor.second.size() == ccsds::HEADER_LEN);
    for (size_t i = 0; i < ccsds::HEADER_LEN; i++) {
      CHECK(accessor.second.data()[i] == buf[i]);
    }
  }

  SECTION("Remove CCSDS header") {}
}
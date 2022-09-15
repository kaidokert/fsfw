#include <catch2/catch_test_macros.hpp>

#include "fsfw/cfdp/CfdpDistributor.h"
#include "fsfw/cfdp/pdu/MetadataPduCreator.h"
#include "fsfw/cfdp/tlv/StringLv.h"
#include "fsfw/storagemanager/LocalPool.h"
#include "fsfw/tcdistribution/definitions.h"
#include "mocks/AcceptsTcMock.h"
#include "mocks/MessageQueueMock.h"
#include "mocks/StorageManagerMock.h"

TEST_CASE("CFDP Distributor", "[cfdp][distributor]") {
  LocalPool::LocalPoolConfig cfg = {{5, 32}, {2, 64}};
  StorageManagerMock pool(objects::NO_OBJECT, cfg);
  auto queue = MessageQueueMock(1);
  CfdpDistribCfg distribCfg(1, pool, &queue);
  auto distributor = CfdpDistributor(distribCfg);
  auto obswEntityId = cfdp::EntityId(UnsignedByteField<uint16_t>(2));
  auto groundEntityId = cfdp::EntityId(UnsignedByteField<uint16_t>(1));
  MessageQueueId_t receiverQueueId = 3;
  auto tcAcceptor = AcceptsTcMock("CFDP Receiver", 0, receiverQueueId);

  // Set up Metadata PDU for generate test data.
  cfdp::FileSize fileSize(12);
  const cfdp::EntityId& sourceId(groundEntityId);
  const cfdp::EntityId& destId(obswEntityId);
  cfdp::TransactionSeqNum seqNum(UnsignedByteField<uint16_t>(12));
  auto pduConf = PduConfig(sourceId, destId, cfdp::TransmissionMode::UNACKNOWLEDGED, seqNum);
  std::string sourceFileString = "hello.txt";
  cfdp::StringLv sourceFileName(sourceFileString);
  std::string destFileString = "hello2.txt";
  cfdp::StringLv destFileName(destFileString);
  MetadataInfo metadataInfo(false, cfdp::ChecksumType::CRC_32, fileSize, sourceFileName,
                            destFileName);
  MetadataPduCreator creator(pduConf, metadataInfo);
  uint8_t* dataPtr = nullptr;

  SECTION("State") {
    CHECK(distributor.initialize() == returnvalue::OK);
    CHECK(std::strcmp(distributor.getName(), "CFDP Distributor") == 0);
    CHECK(distributor.getIdentifier() == 0);
    CHECK(distributor.getRequestQueue() == queue.getId());
  }

  SECTION("Packet Forwarding") {
    CHECK(distributor.initialize() == returnvalue::OK);
    CHECK(distributor.registerTcDestination(obswEntityId, tcAcceptor) == returnvalue::OK);
    size_t serLen = 0;
    store_address_t storeId;
    CHECK(pool.LocalPool::getFreeElement(&storeId, creator.getSerializedSize(), &dataPtr) ==
          returnvalue::OK);
    REQUIRE(creator.SerializeIF::serializeBe(dataPtr, serLen, creator.getSerializedSize()) ==
            returnvalue::OK);
    TmTcMessage msg(storeId);
    queue.addReceivedMessage(msg);
    CHECK(distributor.performOperation(0) == returnvalue::OK);
    CHECK(queue.wasMessageSent());
    CHECK(queue.numberOfSentMessages() == 1);
    // The packet is forwarded, with no need to delete the data
    CHECK(pool.hasDataAtId(storeId));
    TmTcMessage sentMsg;
    CHECK(queue.getNextSentMessage(receiverQueueId, sentMsg) == returnvalue::OK);
    CHECK(sentMsg.getStorageId() == storeId);
  }

  SECTION("No Destination found") {
    CHECK(distributor.initialize() == returnvalue::OK);
    size_t serLen = 0;
    store_address_t storeId;
    CHECK(pool.LocalPool::getFreeElement(&storeId, creator.getSerializedSize(), &dataPtr) ==
          returnvalue::OK);
    REQUIRE(creator.SerializeIF::serializeBe(dataPtr, serLen, creator.getSerializedSize()) ==
            returnvalue::OK);
    TmTcMessage msg(storeId);
    queue.addReceivedMessage(msg);
    CHECK(distributor.performOperation(0) == tmtcdistrib::NO_DESTINATION_FOUND);
  }

  SECTION("Getting data fails") {
    pool.nextModifyDataCallFails.first = true;
    pool.nextModifyDataCallFails.second = StorageManagerIF::DATA_DOES_NOT_EXIST;
    size_t serLen = 0;
    store_address_t storeId;
    CHECK(distributor.registerTcDestination(obswEntityId, tcAcceptor) == returnvalue::OK);
    CHECK(pool.LocalPool::getFreeElement(&storeId, creator.getSerializedSize(), &dataPtr) ==
          returnvalue::OK);
    REQUIRE(creator.SerializeIF::serializeBe(dataPtr, serLen, creator.getSerializedSize()) ==
            returnvalue::OK);
    TmTcMessage msg(storeId);
    queue.addReceivedMessage(msg);
    CHECK(distributor.performOperation(0) == StorageManagerIF::DATA_DOES_NOT_EXIST);
  }

  SECTION("Duplicate registration") {
    CHECK(distributor.initialize() == returnvalue::OK);
    CHECK(distributor.registerTcDestination(obswEntityId, tcAcceptor) == returnvalue::OK);
    CHECK(distributor.registerTcDestination(obswEntityId, tcAcceptor) == returnvalue::FAILED);
  }
}
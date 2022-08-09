#include <catch2/catch_test_macros.hpp>

#include "fsfw/cfdp/CfdpDistributor.h"
#include "fsfw/cfdp/pdu/MetadataPduCreator.h"
#include "fsfw/storagemanager/LocalPool.h"
#include "mocks/AcceptsTcMock.h"
#include "mocks/MessageQueueMock.h"

TEST_CASE("CFDP Distributor", "[cfdp][distributor]") {
  LocalPool::LocalPoolConfig cfg = {{5, 32}, {2, 64}};
  LocalPool pool(objects::NO_OBJECT, cfg);
  auto queue = MessageQueueMock(1);
  CfdpDistribCfg distribCfg(1, pool, &queue);
  auto distributor = CfdpDistributor(distribCfg);
  auto obswEntityId = cfdp::EntityId(UnsignedByteField<uint16_t>(2));
  auto groundEntityId = cfdp::EntityId(UnsignedByteField<uint16_t>(1));
  MessageQueueId_t receiverQueueId = 3;
  auto tcAcceptor = AcceptsTcMock("CFDP Receiver", 0, receiverQueueId);
  cfdp::FileSize fileSize(12);
  const cfdp::EntityId& sourceId(groundEntityId);
  const cfdp::EntityId& destId(obswEntityId);
  cfdp::TransactionSeqNum seqNum(UnsignedByteField<uint16_t>(12));
  auto pduConf = PduConfig(sourceId, destId, cfdp::TransmissionModes::UNACKNOWLEDGED, seqNum);
  std::string sourceFileString = "hello.txt";
  cfdp::Lv sourceFileName(sourceFileString.c_str(), sourceFileString.size());
  std::string destFileString = "hello2.txt";
  cfdp::Lv destFileName(destFileString.c_str(), sourceFileString.size());
  MetadataInfo metadataInfo(false, cfdp::ChecksumType::CRC_32, fileSize, sourceFileName,
                            destFileName);
  MetadataPduCreator creator(pduConf, metadataInfo);
  uint8_t* dataPtr = nullptr;

  SECTION("State") {
    CHECK(distributor.initialize() == result::OK);
    CHECK(std::strcmp(distributor.getName(), "CFDP Distributor") == 0);
    CHECK(distributor.getIdentifier() == 0);
    CHECK(distributor.getRequestQueue() == queue.getId());
  }

  SECTION("Register Listener") {
    CHECK(distributor.initialize() == result::OK);
    CHECK(distributor.registerTcDestination(obswEntityId, tcAcceptor) == result::OK);
    size_t serLen = 0;
    store_address_t storeId;
    CHECK(pool.getFreeElement(&storeId, creator.getSerializedSize(), &dataPtr) == result::OK);
    REQUIRE(creator.SerializeIF::serializeBe(dataPtr, serLen, creator.getSerializedSize()) ==
            result::OK);
    TmTcMessage msg(storeId);
    queue.addReceivedMessage(msg);
    CHECK(distributor.performOperation(0) == result::OK);
    CHECK(queue.wasMessageSent());
    CHECK(queue.numberOfSentMessages() == 1);
    // The packet is forwarded, with no need to delete the data
    CHECK(pool.hasDataAtId(storeId));
    TmTcMessage sentMsg;
    CHECK(queue.getNextSentMessage(receiverQueueId, sentMsg) == result::OK);
    CHECK(sentMsg.getStorageId() == storeId);
  }
}
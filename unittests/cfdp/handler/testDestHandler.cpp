#include <catch2/catch_test_macros.hpp>

#include "fsfw/cfdp.h"
#include "fsfw/cfdp/pdu/MetadataPduCreator.h"
#include "mocks/AcceptsTmMock.h"
#include "mocks/EventReportingProxyMock.h"
#include "mocks/FilesystemMock.h"
#include "mocks/MessageQueueMock.h"
#include "mocks/StorageManagerMock.h"
#include "mocks/cfdp/FaultHandlerMock.h"
#include "mocks/cfdp/RemoteConfigTableMock.h"
#include "mocks/cfdp/UserMock.h"

TEST_CASE("CFDP Dest Handler", "[cfdp]") {
  using namespace cfdp;
  using namespace returnvalue;
  MessageQueueId_t destQueueId = 2;
  AcceptsTmMock tmReceiver(destQueueId);
  MessageQueueMock mqMock(destQueueId);
  EntityId localId = EntityId(UnsignedByteField<uint16_t>(2));
  EntityId remoteId = EntityId(UnsignedByteField<uint16_t>(3));
  auto fhMock = FaultHandlerMock();
  auto localEntityCfg = LocalEntityCfg(localId, IndicationCfg(), fhMock);
  auto fsMock = FilesystemMock();
  auto userMock = UserMock(fsMock);
  auto remoteCfgTableMock = RemoteConfigTableMock();
  PacketInfoList<64> packetInfoList;
  LostSegmentsList<128> lostSegmentsList;
  DestHandlerParams dp(localEntityCfg, userMock, remoteCfgTableMock, packetInfoList,
                       lostSegmentsList);
  EventReportingProxyMock eventReporterMock;
  LocalPool::LocalPoolConfig storeCfg = {{10, 32}, {10, 64}, {10, 128}, {10, 1024}};
  StorageManagerMock tcStore(2, storeCfg);
  StorageManagerMock tmStore(3, storeCfg);
  FsfwParams fp(tmReceiver, mqMock, eventReporterMock);
  RemoteEntityCfg cfg;
  cfg.remoteId = remoteId;
  remoteCfgTableMock.addRemoteConfig(cfg);
  fp.tcStore = &tcStore;
  fp.tmStore = &tmStore;
  auto destHandler = DestHandler(dp, fp);
  CHECK(destHandler.initialize() == OK);

  SECTION("State") {
    CHECK(destHandler.getCfdpState() == CfdpStates::IDLE);
    CHECK(destHandler.getTransactionStep() == DestHandler::TransactionStep::IDLE);
  }

  SECTION("Idle State Machine Iteration") {
    auto res = destHandler.performStateMachine();
    CHECK(res.result == OK);
    CHECK(res.callStatus == CallStatus::CALL_AFTER_DELAY);
    CHECK(res.errors == 0);
    CHECK(destHandler.getCfdpState() == CfdpStates::IDLE);
    CHECK(destHandler.getTransactionStep() == DestHandler::TransactionStep::IDLE);
  }

  SECTION("Empty File Transfer") {
    const DestHandler::FsmResult& res = destHandler.performStateMachine();
    CHECK(res.result == OK);
    FileSize size(0);
    std::string srcNameString = "hello.txt";
    std::string destNameString = "hello-cpy.txt";
    StringLv srcName(srcNameString);
    StringLv destName(destNameString);
    MetadataInfo info(false, cfdp::ChecksumTypes::NULL_CHECKSUM, size, srcName, destName);
    auto seqNum = TransactionSeqNum(UnsignedByteField<uint16_t>(1));
    PduConfig conf(remoteId, localId, TransmissionModes::UNACKNOWLEDGED, seqNum);
    MetadataPduCreator metadataPdu(conf, info);
    store_address_t storeId;
    uint8_t* ptr;
    CHECK(tcStore.getFreeElement(&storeId, metadataPdu.getSerializedSize(), &ptr) == OK);
    size_t serLen = 0;
    CHECK(metadataPdu.serialize(ptr, serLen, metadataPdu.getSerializedSize()) == OK);
    PacketInfo packetInfo(metadataPdu.getPduType(), metadataPdu.getDirectiveCode(), storeId);
    packetInfoList.push_back(packetInfo);
    destHandler.performStateMachine();
    CHECK(res.result == OK);
    CHECK(res.callStatus == CallStatus::CALL_AGAIN);
    destHandler.performStateMachine();
  }

  SECTION("Small File Transfer") {}

  SECTION("Segmented File Transfer") {}
}
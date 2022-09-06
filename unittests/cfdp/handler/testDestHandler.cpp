#include <catch2/catch_test_macros.hpp>

#include "fsfw/cfdp.h"
#include "fsfw/cfdp/pdu/EofPduCreator.h"
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
  FaultHandlerMock fhMock;
  LocalEntityCfg localEntityCfg(localId, IndicationCfg(), fhMock);
  FilesystemMock fsMock;
  UserMock userMock(fsMock);
  RemoteConfigTableMock remoteCfgTableMock;
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
    FileSize cfdpFileSize(0);
    MetadataInfo info(false, cfdp::ChecksumTypes::NULL_CHECKSUM, size, srcName, destName);
    TransactionSeqNum seqNum(UnsignedByteField<uint16_t>(1));
    PduConfig conf(remoteId, localId, TransmissionModes::UNACKNOWLEDGED, seqNum);
    MetadataPduCreator metadataCreator(conf, info);
    store_address_t storeId;
    uint8_t* ptr;
    REQUIRE(tcStore.getFreeElement(&storeId, metadataCreator.getSerializedSize(), &ptr) == OK);
    size_t serLen = 0;
    REQUIRE(metadataCreator.serialize(ptr, serLen, metadataCreator.getSerializedSize()) == OK);
    PacketInfo packetInfo(metadataCreator.getPduType(), metadataCreator.getDirectiveCode(),
                          storeId);
    packetInfoList.push_back(packetInfo);
    destHandler.performStateMachine();
    REQUIRE(res.result == OK);
    REQUIRE(res.callStatus == CallStatus::CALL_AGAIN);
    // Assert that the packet was deleted after handling
    REQUIRE(not tcStore.hasDataAtId(storeId));
    REQUIRE(packetInfoList.empty());
    destHandler.performStateMachine();
    REQUIRE(userMock.metadataRecvd.size() == 1);
    auto& idMetadataPair = userMock.metadataRecvd.back();
    REQUIRE(idMetadataPair.first == destHandler.getTransactionId());
    REQUIRE(idMetadataPair.second.sourceId.getValue() == 3);
    REQUIRE(idMetadataPair.second.fileSize == 0);
    REQUIRE(strcmp(idMetadataPair.second.destFileName, "hello-cpy.txt") == 0);
    REQUIRE(strcmp(idMetadataPair.second.sourceFileName, "hello.txt") == 0);
    userMock.metadataRecvd.pop();
    REQUIRE(fsMock.fileMap.find("hello-cpy.txt") != fsMock.fileMap.end());
    REQUIRE(res.result == OK);
    REQUIRE(res.callStatus == CallStatus::CALL_AFTER_DELAY);
    REQUIRE(res.state == CfdpStates::BUSY_CLASS_1_NACKED);
    REQUIRE(res.step == DestHandler::TransactionStep::RECEIVING_FILE_DATA_PDUS);
    EofInfo eofInfo(cfdp::ConditionCode::NO_ERROR, 0, cfdpFileSize);
    EofPduCreator eofCreator(conf, eofInfo);
    REQUIRE(tcStore.getFreeElement(&storeId, eofCreator.getSerializedSize(), &ptr) == OK);
    REQUIRE(eofCreator.serialize(ptr, serLen, eofCreator.getSerializedSize()) == OK);
    packetInfo = PacketInfo(eofCreator.getPduType(), eofCreator.getDirectiveCode(), storeId);
    packetInfoList.push_back(packetInfo);
    auto transactionId = destHandler.getTransactionId();
    // After EOF, operation is done because no closure was requested
    destHandler.performStateMachine();
    REQUIRE(res.result == OK);
    REQUIRE(res.state == CfdpStates::IDLE);
    REQUIRE(res.step == DestHandler::TransactionStep::IDLE);
    // Assert that the packet was deleted after handling
    REQUIRE(not tcStore.hasDataAtId(storeId));
    REQUIRE(packetInfoList.empty());
    REQUIRE(userMock.eofsRevd.size() == 1);
    auto& eofId = userMock.eofsRevd.back();
    CHECK(eofId == transactionId);
    REQUIRE(userMock.finishedRecvd.size() == 1);
    auto& idParamPair = userMock.finishedRecvd.back();
    CHECK(idParamPair.first == transactionId);
    CHECK(idParamPair.second.condCode == ConditionCode::NO_ERROR);
  }

  SECTION("Small File Transfer") {}

  SECTION("Segmented File Transfer") {}
}
#include <etl/crc32.h>

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
  uint8_t* buf = nullptr;
  size_t serLen = 0;
  auto destHandler = DestHandler(dp, fp);
  CHECK(destHandler.initialize() == OK);

  auto metadataPreparation = [&](PduConfig& conf, FileSize cfdpFileSize, store_address_t& storeId) {
    std::string srcNameString = "hello.txt";
    std::string destNameString = "hello-cpy.txt";
    StringLv srcName(srcNameString);
    StringLv destName(destNameString);
    MetadataInfo info(false, cfdp::ChecksumTypes::NULL_CHECKSUM, cfdpFileSize, srcName, destName);
    TransactionSeqNum seqNum(UnsignedByteField<uint16_t>(1));
    conf.sourceId = remoteId;
    conf.destId = localId;
    conf.mode = TransmissionModes::UNACKNOWLEDGED;
    conf.seqNum = seqNum;
    MetadataPduCreator metadataCreator(conf, info);
    REQUIRE(tcStore.getFreeElement(&storeId, metadataCreator.getSerializedSize(), &buf) == OK);
    REQUIRE(metadataCreator.serialize(buf, serLen, metadataCreator.getSerializedSize()) == OK);
    PacketInfo packetInfo(metadataCreator.getPduType(), metadataCreator.getDirectiveCode(),
                          storeId);
    packetInfoList.push_back(packetInfo);
  };

  auto metadataCheck = [&](const cfdp::DestHandler::FsmResult& res, store_address_t storeId, const char* sourceName, const char* destName) {
    REQUIRE(res.result == OK);
    REQUIRE(res.callStatus == CallStatus::CALL_AGAIN);
    // Assert that the packet was deleted after handling
    REQUIRE(not tcStore.hasDataAtId(storeId));
    REQUIRE(packetInfoList.empty());
    REQUIRE(userMock.metadataRecvd.size() == 1);
    auto& idMetadataPair = userMock.metadataRecvd.back();
    REQUIRE(idMetadataPair.first == destHandler.getTransactionId());
    REQUIRE(idMetadataPair.second.sourceId.getValue() == 3);
    REQUIRE(idMetadataPair.second.fileSize == 0);
    REQUIRE(strcmp(idMetadataPair.second.destFileName, destName) == 0);
    REQUIRE(strcmp(idMetadataPair.second.sourceFileName, sourceName) == 0);
    userMock.metadataRecvd.pop();
    REQUIRE(fsMock.fileMap.find(destName) != fsMock.fileMap.end());
    REQUIRE(res.result == OK);
    REQUIRE(res.state == CfdpStates::BUSY_CLASS_1_NACKED);
    REQUIRE(res.step == DestHandler::TransactionStep::RECEIVING_FILE_DATA_PDUS);
  };

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
    FileSize cfdpFileSize(0);
    store_address_t storeId;
    PduConfig conf;
    metadataPreparation(conf, cfdpFileSize, storeId);
    destHandler.performStateMachine();
    metadataCheck(res, storeId, "hello.txt", "hello-cpy.txt");
    destHandler.performStateMachine();
    REQUIRE(res.callStatus == CallStatus::CALL_AFTER_DELAY);
    EofInfo eofInfo(cfdp::ConditionCode::NO_ERROR, 0, cfdpFileSize);
    EofPduCreator eofCreator(conf, eofInfo);
    REQUIRE(tcStore.getFreeElement(&storeId, eofCreator.getSerializedSize(), &buf) == OK);
    REQUIRE(eofCreator.serialize(buf, serLen, eofCreator.getSerializedSize()) == OK);
    PacketInfo packetInfo(eofCreator.getPduType(), eofCreator.getDirectiveCode(), storeId);
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

  SECTION("Small File Transfer") {
    const DestHandler::FsmResult& res = destHandler.performStateMachine();
    CHECK(res.result == OK);
    std::string fileData = "hello test data";
    etl::crc32 crcCalc;
    crcCalc.add(fileData.begin(), fileData.end());
    uint32_t crc32 = crcCalc.value();
    FileSize cfdpFileSize(0);
    store_address_t storeId;
    PduConfig conf;
    metadataPreparation(conf, cfdpFileSize, storeId);
    destHandler.performStateMachine();
    metadataCheck(res, storeId, "hello.txt", "hello-cpy.txt");
    destHandler.performStateMachine();
    REQUIRE(res.callStatus == CallStatus::CALL_AFTER_DELAY);
  }

  SECTION("Segmented File Transfer") {}
}
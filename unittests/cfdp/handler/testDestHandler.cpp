#include <etl/crc32.h>

#include <catch2/catch_test_macros.hpp>
#include <random>
#include <utility>

#include "fsfw/cfdp.h"
#include "fsfw/cfdp/pdu/EofPduCreator.h"
#include "fsfw/cfdp/pdu/FileDataCreator.h"
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
  FsfwParams fp(tmReceiver, &mqMock, &eventReporterMock);
  RemoteEntityCfg cfg(remoteId);
  remoteCfgTableMock.addRemoteConfig(cfg);
  fp.tcStore = &tcStore;
  fp.tmStore = &tmStore;
  uint8_t* buf = nullptr;
  size_t serLen = 0;
  store_address_t storeId;
  PduConfig conf;
  auto destHandler = DestHandler(dp, fp);
  CHECK(destHandler.initialize() == OK);

  auto metadataPreparation = [&](FileSize cfdpFileSize, ChecksumType checksumType) {
    std::string srcNameString = "hello.txt";
    std::string destNameString = "hello-cpy.txt";
    StringLv srcName(srcNameString);
    StringLv destName(destNameString);
    MetadataInfo info(false, checksumType, cfdpFileSize, srcName, destName);
    TransactionSeqNum seqNum(UnsignedByteField<uint16_t>(1));
    conf.sourceId = remoteId;
    conf.destId = localId;
    conf.mode = TransmissionMode::UNACKNOWLEDGED;
    conf.seqNum = seqNum;
    MetadataPduCreator metadataCreator(conf, info);
    REQUIRE(tcStore.getFreeElement(&storeId, metadataCreator.getSerializedSize(), &buf) == OK);
    REQUIRE(metadataCreator.serialize(buf, serLen, metadataCreator.getSerializedSize()) == OK);
    PacketInfo packetInfo(metadataCreator.getPduType(), storeId,
                          metadataCreator.getDirectiveCode());
    packetInfoList.push_back(packetInfo);
  };

  auto metadataCheck = [&](const cfdp::DestHandler::FsmResult& res, const char* sourceName,
                           const char* destName, size_t fileLen) {
    REQUIRE(res.result == OK);
    REQUIRE(res.callStatus == CallStatus::CALL_AGAIN);
    REQUIRE(res.errors == 0);
    // Assert that the packet was deleted after handling
    REQUIRE(not tcStore.hasDataAtId(storeId));
    REQUIRE(packetInfoList.empty());
    REQUIRE(userMock.metadataRecvd.size() == 1);
    auto& idMetadataPair = userMock.metadataRecvd.back();
    REQUIRE(idMetadataPair.first == destHandler.getTransactionId());
    REQUIRE(idMetadataPair.second.sourceId.getValue() == 3);
    REQUIRE(idMetadataPair.second.fileSize == fileLen);
    REQUIRE(strcmp(idMetadataPair.second.destFileName, destName) == 0);
    REQUIRE(strcmp(idMetadataPair.second.sourceFileName, sourceName) == 0);
    userMock.metadataRecvd.pop();
    REQUIRE(fsMock.fileMap.find(destName) != fsMock.fileMap.end());
    REQUIRE(res.result == OK);
    REQUIRE(res.state == CfdpStates::BUSY_CLASS_1_NACKED);
    REQUIRE(res.step == DestHandler::TransactionStep::RECEIVING_FILE_DATA_PDUS);
  };

  auto eofPreparation = [&](FileSize cfdpFileSize, uint32_t crc) {
    EofInfo eofInfo(cfdp::ConditionCode::NO_ERROR, crc, std::move(cfdpFileSize));
    EofPduCreator eofCreator(conf, eofInfo);
    REQUIRE(tcStore.getFreeElement(&storeId, eofCreator.getSerializedSize(), &buf) == OK);
    REQUIRE(eofCreator.serialize(buf, serLen, eofCreator.getSerializedSize()) == OK);
    PacketInfo packetInfo(eofCreator.getPduType(), storeId, eofCreator.getDirectiveCode());
    packetInfoList.push_back(packetInfo);
  };

  auto eofCheck = [&](const cfdp::DestHandler::FsmResult& res, const TransactionId& id) {
    REQUIRE(res.result == OK);
    REQUIRE(res.state == CfdpStates::IDLE);
    REQUIRE(res.errors == 0);
    REQUIRE(res.step == DestHandler::TransactionStep::IDLE);
    // Assert that the packet was deleted after handling
    REQUIRE(not tcStore.hasDataAtId(storeId));
    REQUIRE(packetInfoList.empty());
    REQUIRE(userMock.eofsRevd.size() == 1);
    auto& eofId = userMock.eofsRevd.back();
    CHECK(eofId == id);
    REQUIRE(userMock.finishedRecvd.size() == 1);
    auto& idParamPair = userMock.finishedRecvd.back();
    CHECK(idParamPair.first == id);
    CHECK(idParamPair.second.condCode == ConditionCode::NO_ERROR);
  };

  auto fileDataPduCheck = [&](const cfdp::DestHandler::FsmResult& res,
                              const std::vector<store_address_t>& idsToCheck) {
    REQUIRE(res.result == OK);
    REQUIRE(res.state == CfdpStates::BUSY_CLASS_1_NACKED);
    REQUIRE(res.step == DestHandler::TransactionStep::RECEIVING_FILE_DATA_PDUS);
    for (const auto id : idsToCheck) {
      REQUIRE(not tcStore.hasDataAtId(id));
    }
    REQUIRE(packetInfoList.empty());
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
    metadataPreparation(cfdpFileSize, ChecksumType::NULL_CHECKSUM);
    destHandler.performStateMachine();
    metadataCheck(res, "hello.txt", "hello-cpy.txt", 0);
    destHandler.performStateMachine();
    REQUIRE(res.callStatus == CallStatus::CALL_AFTER_DELAY);
    auto transactionId = destHandler.getTransactionId();
    eofPreparation(cfdpFileSize, 0);
    // After EOF, operation is done because no closure was requested
    destHandler.performStateMachine();
    eofCheck(res, transactionId);
  }

  SECTION("Small File Transfer") {
    const DestHandler::FsmResult& res = destHandler.performStateMachine();
    CHECK(res.result == OK);
    std::string fileData = "hello test data";
    etl::crc32 crcCalc;
    crcCalc.add(fileData.begin(), fileData.end());
    uint32_t crc32 = crcCalc.value();
    FileSize cfdpFileSize(fileData.size());
    metadataPreparation(cfdpFileSize, ChecksumType::CRC_32);
    destHandler.performStateMachine();
    metadataCheck(res, "hello.txt", "hello-cpy.txt", fileData.size());
    destHandler.performStateMachine();
    REQUIRE(res.callStatus == CallStatus::CALL_AFTER_DELAY);
    auto transactionId = destHandler.getTransactionId();
    FileSize offset(0);
    FileDataInfo fdPduInfo(offset, reinterpret_cast<const uint8_t*>(fileData.data()),
                           fileData.size());
    FileDataCreator fdPduCreator(conf, fdPduInfo);
    REQUIRE(tcStore.getFreeElement(&storeId, fdPduCreator.getSerializedSize(), &buf) == OK);
    REQUIRE(fdPduCreator.serialize(buf, serLen, fdPduCreator.getSerializedSize()) == OK);
    PacketInfo packetInfo(fdPduCreator.getPduType(), storeId, std::nullopt);
    packetInfoList.push_back(packetInfo);
    destHandler.performStateMachine();
    fileDataPduCheck(res, {storeId});
    eofPreparation(cfdpFileSize, crc32);
    // After EOF, operation is done because no closure was requested
    destHandler.performStateMachine();
    eofCheck(res, transactionId);
  }

  SECTION("Segmented File Transfer") {
    const DestHandler::FsmResult& res = destHandler.performStateMachine();
    CHECK(res.result == OK);
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> distU8(0, 255);
    std::array<uint8_t, 1024> largerFileData{};
    for (auto& val : largerFileData) {
      val = distU8(rng);
    }
    etl::crc32 crcCalc;
    crcCalc.add(largerFileData.begin(), largerFileData.end());
    uint32_t crc32 = crcCalc.value();
    FileSize cfdpFileSize(largerFileData.size());
    metadataPreparation(cfdpFileSize, ChecksumType::CRC_32);
    destHandler.performStateMachine();
    metadataCheck(res, "hello.txt", "hello-cpy.txt", largerFileData.size());
    destHandler.performStateMachine();
    REQUIRE(res.callStatus == CallStatus::CALL_AFTER_DELAY);
    auto transactionId = destHandler.getTransactionId();

    std::vector<store_address_t> idsToCheck;
    {
      FileSize offset(0);
      FileDataInfo fdPduInfo(offset, reinterpret_cast<const uint8_t*>(largerFileData.data()),
                             largerFileData.size() / 2);
      FileDataCreator fdPduCreator(conf, fdPduInfo);
      REQUIRE(tcStore.getFreeElement(&storeId, fdPduCreator.getSerializedSize(), &buf) == OK);
      REQUIRE(fdPduCreator.serialize(buf, serLen, fdPduCreator.getSerializedSize()) == OK);
      PacketInfo packetInfo(fdPduCreator.getPduType(), storeId, std::nullopt);
      idsToCheck.push_back(storeId);
      packetInfoList.push_back(packetInfo);
    }

    {
      FileSize offset(512);
      FileDataInfo fdPduInfo(offset, reinterpret_cast<const uint8_t*>(largerFileData.data() + 512),
                             largerFileData.size() / 2);
      FileDataCreator fdPduCreator(conf, fdPduInfo);
      REQUIRE(tcStore.getFreeElement(&storeId, fdPduCreator.getSerializedSize(), &buf) == OK);
      REQUIRE(fdPduCreator.serialize(buf, serLen, fdPduCreator.getSerializedSize()) == OK);
      PacketInfo packetInfo(fdPduCreator.getPduType(), storeId, std::nullopt);
      idsToCheck.push_back(storeId);
      packetInfoList.push_back(packetInfo);
    }

    destHandler.performStateMachine();
    fileDataPduCheck(res, idsToCheck);
    eofPreparation(cfdpFileSize, crc32);
    // After EOF, operation is done because no closure was requested
    destHandler.performStateMachine();
    eofCheck(res, transactionId);
  }
}
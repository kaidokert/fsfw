#include <catch2/catch_test_macros.hpp>

#include "fsfw/cfdp.h"
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
  fp.tcStore = &tcStore;
  fp.tmStore = &tmStore;
  auto destHandler = DestHandler(dp, fp);
  CHECK(destHandler.initialize() == OK);

  SECTION("State") {
    CHECK(destHandler.getCfdpState() == CfdpStates::IDLE);
    CHECK(destHandler.getTransactionStep() == DestHandler::TransactionStep::IDLE);
  }
}
#include <catch2/catch_test_macros.hpp>

#include "fsfw/cfdp.h"
#include "mocks/AcceptsTmMock.h"
#include "mocks/FilesystemMock.h"
#include "mocks/MessageQueueMock.h"
#include "mocks/cfdp/FaultHandlerMock.h"
#include "mocks/cfdp/RemoteConfigTableMock.h"
#include "mocks/cfdp/UserMock.h"

TEST_CASE("CFDP Dest Handler", "[cfdp]") {
  using namespace cfdp;
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
  // FsfwParams fp(destQueueId, mqMock);
  // auto destHandler = DestHandler();

  SECTION("State") {}
}
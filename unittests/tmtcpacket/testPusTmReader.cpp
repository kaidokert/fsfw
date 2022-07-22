#include <catch2/catch_test_macros.hpp>

#include "fsfw/tmtcpacket/pus/tm/PusTmCreator.h"
#include "mocks/CdsShortTimestamperMock.h"

TEST_CASE("PUS TM Reader", "[pus-tm-reader]") {
  auto packetId = PacketId(ccsds::PacketType::TC, true, 0xef);
  auto spParams =
      SpacePacketParams(packetId, PacketSeqCtrl(ccsds::SequenceFlags::UNSEGMENTED, 22), 0x00);
  auto timeStamper = CdsShortTimestamperMock();
  auto pusTmParams = PusTmParams(17, 2, &timeStamper);
  timeStamper.valueToStamp = {1, 2, 3, 4, 5, 6, 7};
  PusTmCreator creator(spParams, pusTmParams);

  SECTION("State") {
    
  }
}
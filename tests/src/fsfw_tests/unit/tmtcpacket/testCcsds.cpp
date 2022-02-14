#include <catch2/catch_test_macros.hpp>

#include "fsfw/tmtcpacket/SpacePacket.h"

TEST_CASE("CCSDS Test", "[ccsds]") {
  REQUIRE(spacepacket::getTcSpacePacketIdFromApid(0x22) == 0x1822);
  REQUIRE(spacepacket::getTmSpacePacketIdFromApid(0x22) == 0x0822);

  REQUIRE(spacepacket::getTcSpacePacketIdFromApid(0x7ff) == 0x1fff);
  REQUIRE(spacepacket::getTmSpacePacketIdFromApid(0x7ff) == 0xfff);
}

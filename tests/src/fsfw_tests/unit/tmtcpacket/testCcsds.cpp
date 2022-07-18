#include <catch2/catch_test_macros.hpp>

#include "fsfw/tmtcpacket/SpacePacketBase.h"

TEST_CASE("CCSDS Test", "[ccsds]") {
  REQUIRE(ccsds::getTcSpacePacketIdFromApid(0x22) == 0x1822);
  REQUIRE(ccsds::getTmSpacePacketIdFromApid(0x22) == 0x0822);

  REQUIRE(ccsds::getTcSpacePacketIdFromApid(0x7ff) == 0x1fff);
  REQUIRE(ccsds::getTmSpacePacketIdFromApid(0x7ff) == 0xfff);
}

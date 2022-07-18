#include <catch2/catch_test_macros.hpp>

#include "fsfw/tmtcpacket/SpacePacketReader.h"

TEST_CASE("CCSDS Test", "[ccsds]") {
  SECTION("Constexpr Helpers") {
    REQUIRE(ccsds::getTcSpacePacketIdFromApid(0x22) == 0x1822);
    REQUIRE(ccsds::getTmSpacePacketIdFromApid(0x22) == 0x0822);

    REQUIRE(ccsds::getTcSpacePacketIdFromApid(0x7ff) == 0x1fff);
    REQUIRE(ccsds::getTmSpacePacketIdFromApid(0x7ff) == 0xfff);
  }

  SECTION("Header Tests") { SpacePacketReader base = SpacePacketReader(); }
}

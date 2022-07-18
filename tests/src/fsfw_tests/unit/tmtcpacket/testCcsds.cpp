#include <catch2/catch_test_macros.hpp>

#include "fsfw/tmtcpacket/SpacePacketCreator.h"
#include "fsfw/tmtcpacket/SpacePacketReader.h"

TEST_CASE("CCSDS Test", "[ccsds]") {
  SECTION("Constexpr Helpers") {
    REQUIRE(ccsds::getTcSpacePacketIdFromApid(0x22) == 0x1822);
    REQUIRE(ccsds::getTmSpacePacketIdFromApid(0x22) == 0x0822);

    REQUIRE(ccsds::getTcSpacePacketIdFromApid(0x7ff) == 0x1fff);
    REQUIRE(ccsds::getTmSpacePacketIdFromApid(0x7ff) == 0xfff);
  }

  SECTION("Header Creator Tests") {
    SpacePacketCreator base = SpacePacketCreator(
        ccsds::PacketType::TC, true, 0x02,
        ccsds::SequenceFlags::FIRST_SEGMENT,  0x34, 0x16);
    REQUIRE(base.getApid() == 0x02);
    REQUIRE(base.getSequenceFlags() == ccsds::SequenceFlags::FIRST_SEGMENT);
    REQUIRE(base.getVersion() == 0b000);
    REQUIRE(base.getSequenceCount() == 0x34);
    REQUIRE(base.getPacketDataLen() == 0x16);
    REQUIRE(base.getPacketType() == ccsds::PacketType::TC);
  }
}

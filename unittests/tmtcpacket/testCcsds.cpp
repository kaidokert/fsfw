#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/tmtcpacket/SpacePacketCreator.h"
#include "fsfw/tmtcpacket/SpacePacketReader.h"

TEST_CASE("CCSDS Test", "[ccsds]") {
  SpacePacketCreator base = SpacePacketCreator(
      ccsds::PacketType::TC, true, 0x02,
      ccsds::SequenceFlags::FIRST_SEGMENT,  0x34, 0x16);

  SECTION("Constexpr Helpers") {
    REQUIRE(ccsds::getTcSpacePacketIdFromApid(0x22) == 0x1822);
    REQUIRE(ccsds::getTmSpacePacketIdFromApid(0x22) == 0x0822);

    REQUIRE(ccsds::getTcSpacePacketIdFromApid(0x7ff) == 0x1fff);
    REQUIRE(ccsds::getTmSpacePacketIdFromApid(0x7ff) == 0xfff);
  }

  SECTION("Basic Test") {
    REQUIRE(base.isValid());
    REQUIRE(base.getApid() == 0x02);
    REQUIRE(base.getSequenceFlags() == ccsds::SequenceFlags::FIRST_SEGMENT);
    REQUIRE(base.getVersion() == 0b000);
    REQUIRE(base.getSequenceCount() == 0x34);
    REQUIRE(base.getPacketDataLen() == 0x16);
    REQUIRE(base.getPacketType() == ccsds::PacketType::TC);
    REQUIRE(base.getPacketId() == 0x1802);
  }

  SECTION("Raw Output") {
    std::array<uint8_t, 6> buf {};
    uint8_t* bufPtr = buf.data();
    size_t serLen = 0;
    base.serialize(&bufPtr, &serLen, buf.size(),  SerializeIF::Endianness::MACHINE);
    REQUIRE(buf[0] == 0x18);
    REQUIRE(buf[1] == 0x02);
    REQUIRE(buf[2] == 0x40);
    REQUIRE(buf[3] == 0x34);
    REQUIRE(buf[4] == 0x00);
    REQUIRE(buf[5] == 0x16);
  }
}

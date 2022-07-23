#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cmath>

#include "fsfw/tmtcpacket/ccsds/SpacePacketCreator.h"

TEST_CASE("CCSDS Creator", "[ccsds-creator]") {
  auto params = SpacePacketParams(PacketId(ccsds::PacketType::TC, true, 0x02),
                                  PacketSeqCtrl(ccsds::SequenceFlags::FIRST_SEGMENT, 0x34), 0x16);
  SpacePacketCreator base = SpacePacketCreator(params);
  std::array<uint8_t, 6> buf{};
  uint8_t* bufPtr = buf.data();
  size_t serLen = 0;

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
    REQUIRE(base.getPacketIdRaw() == 0x1802);
    REQUIRE(base.getSerializedSize() == 6);
  }

  SECTION("Deserialization Fails") {
    serLen = 6;
    const uint8_t* readOnlyPtr = buf.data();
    REQUIRE(base.deSerialize(&readOnlyPtr, &serLen, SerializeIF::Endianness::BIG) ==
            HasReturnvaluesIF::RETURN_FAILED);
  }

  SECTION("Raw Output") {
    REQUIRE(base.serializeNe(&bufPtr, &serLen, buf.size()) == HasReturnvaluesIF::RETURN_OK);
    // TC, and secondary header flag is set -> 0b0001100 -> 0x18
    REQUIRE(buf[0] == 0x18);
    // APID 0x02
    REQUIRE(buf[1] == 0x02);
    // Sequence count is one byte value, so the only set bit here is the bit
    // from the Sequence flag argument, which is the second bit for
    // SequenceFlags.FIRST_SEGMENT
    REQUIRE(buf[2] == 0x40);
    // Sequence Count specified above
    REQUIRE(buf[3] == 0x34);
    // This byte and the next byte should be 22 big endian (packet length)
    REQUIRE(buf[4] == 0x00);
    REQUIRE(buf[5] == 0x16);
  }

  SECTION("All Ones Output") {
    base.setApid(static_cast<int>(std::pow(2, 11)) - 1);
    base.setSeqCount(static_cast<int>(std::pow(2, 14)) - 1);
    base.setSeqFlags(ccsds::SequenceFlags::UNSEGMENTED);
    base.setDataLen(static_cast<int>(std::pow(2, 16)) - 1);
    REQUIRE(base.isValid());
    REQUIRE(base.serializeNe(&bufPtr, &serLen, buf.size()) == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(buf[0] == 0x1F);
    REQUIRE(buf[1] == 0xFF);
    REQUIRE(buf[2] == 0xFF);
    REQUIRE(buf[3] == 0xFF);
    REQUIRE(buf[4] == 0xFF);
    REQUIRE(buf[5] == 0xFF);
  }

  SECTION("Invalid APID") {
    SpacePacketCreator invalid = SpacePacketCreator(
        ccsds::PacketType::TC, true, 0xFFFF, ccsds::SequenceFlags::FIRST_SEGMENT, 0x34, 0x16);
    REQUIRE(not invalid.isValid());
    REQUIRE(invalid.serializeNe(&bufPtr, &serLen, buf.size()) == HasReturnvaluesIF::RETURN_FAILED);
  }

  SECTION("Invalid Seq Count") {
    SpacePacketCreator invalid = SpacePacketCreator(
        ccsds::PacketType::TC, true, 0x02, ccsds::SequenceFlags::FIRST_SEGMENT, 0xFFFF, 0x16);
    REQUIRE(not invalid.isValid());
    REQUIRE(invalid.serializeNe(&bufPtr, &serLen, buf.size()) == HasReturnvaluesIF::RETURN_FAILED);
  }

  SECTION("Invalid Buf Size 1") {
    serLen = 2;
    REQUIRE(base.serializeNe(&bufPtr, &serLen, buf.size()) == SerializeIF::BUFFER_TOO_SHORT);
  }

  SECTION("Invalid Buf Size 2") {
    serLen = 4;
    REQUIRE(base.serializeNe(&bufPtr, &serLen, buf.size()) == SerializeIF::BUFFER_TOO_SHORT);
  }

  SECTION("Invalid Buf Size 3") {
    serLen = 6;
    REQUIRE(base.serializeNe(&bufPtr, &serLen, buf.size()) == SerializeIF::BUFFER_TOO_SHORT);
  }
}

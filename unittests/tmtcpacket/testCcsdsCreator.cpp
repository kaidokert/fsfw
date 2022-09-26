#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cmath>

#include "fsfw/tmtcpacket/ccsds/SpacePacketCreator.h"

TEST_CASE("CCSDS Creator", "[ccsds-creator]") {
  auto packetId = PacketId(ccsds::PacketType::TC, true, 0x02);
  auto psc = PacketSeqCtrl(ccsds::SequenceFlags::FIRST_SEGMENT, 0x34);
  auto params = SpacePacketParams(packetId, psc, 0x16);
  SpacePacketCreator base = SpacePacketCreator(params);
  std::array<uint8_t, 6> buf{};
  uint8_t* bufPtr = buf.data();
  size_t serLen = 0;

  SECTION("Constexpr Helpers") {
    REQUIRE(ccsds::getTcSpacePacketIdFromApid(0x22, true) == 0x1822);
    REQUIRE(ccsds::getTmSpacePacketIdFromApid(0x22, true) == 0x0822);

    REQUIRE(ccsds::getTcSpacePacketIdFromApid(0x22, false) == 0x1022);
    REQUIRE(ccsds::getTmSpacePacketIdFromApid(0x22, false) == 0x0022);

    REQUIRE(ccsds::getTcSpacePacketIdFromApid(0x7ff, true) == 0x1fff);
    REQUIRE(ccsds::getTmSpacePacketIdFromApid(0x7ff, true) == 0xfff);

    REQUIRE(ccsds::getTcSpacePacketIdFromApid(0x7ff, false) == 0x17ff);
    REQUIRE(ccsds::getTmSpacePacketIdFromApid(0x7ff, false) == 0x7ff);
  }

  SECTION("Basic Test") {
    CHECK(base.isValid());
    CHECK(base);
    CHECK(base.getApid() == 0x02);
    CHECK(base.getSequenceFlags() == ccsds::SequenceFlags::FIRST_SEGMENT);
    CHECK(base.getVersion() == 0b000);
    CHECK(base.getSequenceCount() == 0x34);
    CHECK(base.getPacketDataLen() == 0x16);
    CHECK(base.getPacketType() == ccsds::PacketType::TC);
    CHECK(base.getPacketIdRaw() == 0x1802);
    CHECK(base.getSerializedSize() == 6);
    CHECK(base.getPacketSeqCtrl() == psc);
    CHECK(base.getPacketId() == packetId);
  }

  SECTION("Deserialization Fails") {
    serLen = 6;
    const uint8_t* readOnlyPtr = buf.data();
    REQUIRE(base.deSerialize(&readOnlyPtr, &serLen, SerializeIF::Endianness::BIG) ==
            returnvalue::FAILED);
  }

  SECTION("Raw Output") {
    REQUIRE(base.serializeBe(&bufPtr, &serLen, buf.size()) == returnvalue::OK);
    // TC, and secondary header flag is set -> 0b0001100 -> 0x18
    CHECK(buf[0] == 0x18);
    // APID 0x02
    CHECK(buf[1] == 0x02);
    // Sequence count is one byte value, so the only set bit here is the bit
    // from the Sequence flag argument, which is the second bit for
    // SequenceFlags.FIRST_SEGMENT
    CHECK(buf[2] == 0x40);
    // Sequence Count specified above
    CHECK(buf[3] == 0x34);
    // This byte and the next byte should be 22 big endian (packet length)
    CHECK(buf[4] == 0x00);
    CHECK(buf[5] == 0x16);
  }

  SECTION("All Ones Output") {
    base.setApid(static_cast<int>(std::pow(2, 11)) - 1);
    base.setSeqCount(static_cast<int>(std::pow(2, 14)) - 1);
    base.setSeqFlags(ccsds::SequenceFlags::UNSEGMENTED);
    base.setDataLen(static_cast<int>(std::pow(2, 16)) - 1);
    REQUIRE(base.isValid());
    REQUIRE(base.serializeBe(&bufPtr, &serLen, buf.size()) == returnvalue::OK);
    CHECK(buf[0] == 0x1F);
    CHECK(buf[1] == 0xFF);
    CHECK(buf[2] == 0xFF);
    CHECK(buf[3] == 0xFF);
    CHECK(buf[4] == 0xFF);
    CHECK(buf[5] == 0xFF);
  }

  SECTION("Invalid APID") {
    SpacePacketCreator creator = SpacePacketCreator(
        ccsds::PacketType::TC, true, 0xFFFF, ccsds::SequenceFlags::FIRST_SEGMENT, 0x34, 0x16);
    REQUIRE(not creator.isValid());
    REQUIRE(not creator);
    REQUIRE(creator.serializeBe(&bufPtr, &serLen, buf.size()) == returnvalue::FAILED);
  }

  SECTION("Invalid Seq Count") {
    SpacePacketCreator invalid = SpacePacketCreator(
        ccsds::PacketType::TC, true, 0x02, ccsds::SequenceFlags::FIRST_SEGMENT, 0xFFFF, 0x16);
    REQUIRE(not invalid.isValid());
    REQUIRE(invalid.serializeBe(&bufPtr, &serLen, buf.size()) == returnvalue::FAILED);
  }

  SECTION("Invalid Buf Size 1") {
    serLen = 2;
    REQUIRE(base.serializeBe(&bufPtr, &serLen, buf.size()) == SerializeIF::BUFFER_TOO_SHORT);
  }

  SECTION("Invalid Buf Size 2") {
    serLen = 4;
    REQUIRE(base.serializeBe(&bufPtr, &serLen, buf.size()) == SerializeIF::BUFFER_TOO_SHORT);
  }

  SECTION("Invalid Buf Size 3") {
    serLen = 6;
    REQUIRE(base.serializeBe(&bufPtr, &serLen, buf.size()) == SerializeIF::BUFFER_TOO_SHORT);
  }
}

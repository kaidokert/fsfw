#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cmath>

#include "fsfw/tmtcpacket/ccsds/PacketId.h"
#include "fsfw/tmtcpacket/ccsds/PacketSeqCtrl.h"
#include "fsfw/tmtcpacket/ccsds/header.h"

TEST_CASE("CCSDS Low Level", "[ccsds-ll]") {
  SECTION("Lowlevel Header Packet ID test") {
    ccsds::PrimaryHeader header{};
    uint16_t packetIdRaw = 0x3ff;
    ccsds::setPacketId(header, packetIdRaw);
    REQUIRE(header.packetIdHAndVersion == 3);
    REQUIRE(header.packetIdL == 0xff);
    REQUIRE(ccsds::getPacketId(header) == 0x3ff);
    header.packetIdHAndVersion |= 0b00100000;
    REQUIRE(ccsds::getPacketId(header) == 0x3ff);
    REQUIRE(ccsds::getVersion(header) == 0b001);
  }

  SECTION("Lowlevel Header APID Test") {
    ccsds::PrimaryHeader header{};
    uint16_t packetIdRaw = 0x3ff;
    ccsds::setPacketId(header, packetIdRaw);
    ccsds::setApid(header, 0x1ff);
    REQUIRE(ccsds::getPacketId(header) == 0x3ff);
  }

  SECTION("Lowlevel Packet Length Test") {
    ccsds::PrimaryHeader header{};
    header.packetLenH = 0x02;
    header.packetLenL = 0x03;
    REQUIRE(ccsds::getPacketLen(header) == 0x0203);
  }
}

TEST_CASE("CCSDS Packet ID", "[ccsds-packet-id]") {
  PacketId packetId;
  std::array<uint8_t, 3> buf{};
  SECTION("Basic") {
    packetId.apid = 0x1ff;
    packetId.secHeaderFlag = false;
    packetId.packetType = ccsds::PacketType::TM;
    size_t serLen = 0;
    REQUIRE(packetId.raw() == 0x1ff);
    REQUIRE(packetId.SerializeIF::serializeBe(buf.data(), serLen, buf.size()) == returnvalue::OK);
    CHECK(buf[0] == 0x1);
    CHECK(buf[1] == 0xff);
  }

  SECTION("Invalid Ser") {
    size_t serLen = 0;
    REQUIRE(packetId.SerializeIF::serializeBe(buf.data(), serLen, 0) ==
            SerializeIF::BUFFER_TOO_SHORT);
    REQUIRE(packetId.SerializeIF::serializeBe(buf.data(), serLen, 1) ==
            SerializeIF::BUFFER_TOO_SHORT);
  }

  SECTION("Invalid Deser") {
    size_t deserLen = 0;
    REQUIRE(packetId.deSerialize(buf.data(), deserLen, 1, SerializeIF::Endianness::NETWORK) ==
            SerializeIF::STREAM_TOO_SHORT);
    REQUIRE(packetId.deSerialize(buf.data(), deserLen, 0, SerializeIF::Endianness::NETWORK) ==
            SerializeIF::STREAM_TOO_SHORT);
  }

  SECTION("From Raw") {
    auto newId = PacketId(ccsds::PacketType::TC, true, 0x2ff);
    uint16_t rawId = newId.raw();
    CHECK(rawId == 0x1aff);
    CHECK(PacketId::fromRaw(rawId) == newId);
  }

  SECTION("Deserialize") {
    buf[0] = 0x1a;
    buf[1] = 0xff;
    size_t deserLen = 0xff;
    REQUIRE(packetId.deSerialize(buf.data(), deserLen, buf.size(),
                                 SerializeIF::Endianness::NETWORK) == returnvalue::OK);
    CHECK(packetId.apid == 0x2ff);
    CHECK(deserLen == 2);
    CHECK(packetId.packetType == ccsds::PacketType::TC);
    CHECK(packetId.secHeaderFlag == true);
  }
}

TEST_CASE("CCSDS Packet Seq Ctrl", "[ccsds-packet-seq-ctrl]") {
  PacketSeqCtrl psc;
  std::array<uint8_t, 3> buf{};
  SECTION("Basic") {
    size_t serLen = 0xff;
    psc.seqFlags = ccsds::SequenceFlags::FIRST_SEGMENT;
    psc.seqCount = static_cast<uint16_t>(std::round(std::pow(2, 14) - 1));
    REQUIRE(psc.raw() == 0x7fff);
    REQUIRE(psc.SerializeIF::serializeBe(buf.data(), serLen, buf.size()) == returnvalue::OK);
    CHECK(buf[0] == 0x7f);
    CHECK(buf[1] == 0xff);
    CHECK(serLen == 2);
  }

  SECTION("From Raw") {
    auto newId = PacketSeqCtrl(ccsds::SequenceFlags::LAST_SEGMENT,
                               static_cast<uint16_t>(std::round(std::pow(2, 14) - 2)));
    uint16_t rawId = newId.raw();
    REQUIRE(rawId == 0xbffe);
    CHECK(PacketSeqCtrl::fromRaw(rawId) == newId);
  }

  SECTION("Deserialize") {
    buf[0] = 0xbf;
    buf[1] = 0xfe;
    size_t deserLen = 0xff;
    REQUIRE(psc.deSerialize(buf.data(), deserLen, buf.size(), SerializeIF::Endianness::NETWORK) ==
            returnvalue::OK);
    CHECK(psc.seqFlags == ccsds::SequenceFlags::LAST_SEGMENT);
    CHECK(deserLen == 2);
    CHECK(psc.seqCount == static_cast<uint16_t>(std::round(std::pow(2, 14) - 2)));
  }

  SECTION("Invalid Ser") {
    size_t deserLen = 0;
    REQUIRE(psc.SerializeIF::serializeBe(buf.data(), deserLen, 0) == SerializeIF::BUFFER_TOO_SHORT);
    REQUIRE(psc.SerializeIF::serializeBe(buf.data(), deserLen, 1) == SerializeIF::BUFFER_TOO_SHORT);
  }

  SECTION("Invalid Deser") {
    size_t deserLen = 0;
    REQUIRE(psc.deSerialize(buf.data(), deserLen, 1, SerializeIF::Endianness::NETWORK) ==
            SerializeIF::STREAM_TOO_SHORT);
    REQUIRE(psc.deSerialize(buf.data(), deserLen, 0, SerializeIF::Endianness::NETWORK) ==
            SerializeIF::STREAM_TOO_SHORT);
  }
}

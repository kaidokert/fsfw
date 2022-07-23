#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/tmtcpacket/ccsds/PacketId.h"
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
    REQUIRE(ccsds::getPacketId(header) == 0x1ff);
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
    REQUIRE(packetId.raw() == 0x1ff);
    REQUIRE(packetId.SerializeIF::serializeNe(buf.data(), buf.size()) ==
            HasReturnvaluesIF::RETURN_OK);
    REQUIRE(buf[0] == 0x1);
    REQUIRE(buf[1] == 0xff);
  }

  SECTION("From Raw") {
    auto newId = PacketId(ccsds::PacketType::TC, true, 0x2ff);
    uint16_t rawId = newId.raw();
    REQUIRE(rawId == 0x1aff);
    REQUIRE(PacketId::fromRaw(rawId) == newId);
  }
}

TEST_CASE("CCSDS Packet Seq Ctrl", "[ccsds-packet-seq-ctrl]") {}

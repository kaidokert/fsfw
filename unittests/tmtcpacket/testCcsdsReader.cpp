#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/tmtcpacket/ccsds/SpacePacketCreator.h"
#include "fsfw/tmtcpacket/ccsds/SpacePacketReader.h"

#define FULL_PACKET_LEN 29

TEST_CASE("CCSDS Reader", "[ccsds-reader]") {
  auto params = SpacePacketParams(PacketId(ccsds::PacketType::TC, true, 0x02),
                                  PacketSeqCtrl(ccsds::SequenceFlags::FIRST_SEGMENT, 0x34), 0x16);
  SpacePacketCreator base = SpacePacketCreator(params);
  // This is enough to hold 0x16 (22) bytes + 6 (SP header length) + 1 as defined as the full packet
  // length derived from the length field
  std::array<uint8_t, FULL_PACKET_LEN> buf{};
  uint8_t* bufPtr = buf.data();
  size_t serLen = 0;
  SpacePacketReader reader;

  auto checkReader = [&](SpacePacketReader& reader) {
    REQUIRE(reader.getPacketDataLen() == 0x16);
    REQUIRE(reader.getBufSize() == SpacePacketIF::getHeaderLen());
    REQUIRE(reader.getFullData() == buf.data());
    REQUIRE(reader.getFullPacketLen() == 0x16 + SpacePacketReader::getHeaderLen() + 1);
    REQUIRE(reader.getPacketIdRaw() == 0x1802);
    REQUIRE(reader.getSequenceFlags() == ccsds::SequenceFlags::FIRST_SEGMENT);
    REQUIRE(reader.getSequenceCount() == 0x34);
    REQUIRE(reader.getPacketSeqCtrlRaw() == 0x4034);
    REQUIRE(reader.hasSecHeader());
    REQUIRE(reader.getApid() == 0x02);
    REQUIRE(not reader.isNull());
    // We only serialized the 6 bytes of the header, so the packer data should be invalid
    REQUIRE(reader.getPacketData() == nullptr);
  };

  SECTION("Empty Reader") {
    REQUIRE(SpacePacketIF::getHeaderLen() == 6);
    REQUIRE(reader.isNull());
    REQUIRE(reader.checkSize() == returnvalue::FAILED);
  }

  SECTION("Basic Read") {
    REQUIRE(base.serialize(&bufPtr, &serLen, buf.size(), SerializeIF::Endianness::NETWORK) ==
            returnvalue::OK);
    SECTION("Setter") {
      reader.setReadOnlyData(buf.data(), SpacePacketIF::getHeaderLen());
      checkReader(reader);
    }
    SECTION("Direct Construction") {
      SpacePacketReader secondReader(buf.data(), serLen);
      checkReader(secondReader);
    }
  }

  SECTION("Read with additional data") {
    REQUIRE(base.serialize(&bufPtr, &serLen, buf.size(), SerializeIF::Endianness::NETWORK) ==
            returnvalue::OK);
    REQUIRE(reader.setReadOnlyData(buf.data(), buf.size()) == returnvalue::OK);
    REQUIRE(reader.getBufSize() == buf.size());
    REQUIRE(reader.getFullPacketLen() == FULL_PACKET_LEN);
    REQUIRE(reader.getPacketData() == buf.data() + SpacePacketIF::getHeaderLen());
  }

  SECTION("Invalid Size") {
    for (size_t i = 0; i < 5; i++) {
      REQUIRE(reader.setReadOnlyData(buf.data(), i) == SerializeIF::STREAM_TOO_SHORT);
      REQUIRE(reader.isNull());
      REQUIRE(reader.getPacketData() == nullptr);
    }
  }
}
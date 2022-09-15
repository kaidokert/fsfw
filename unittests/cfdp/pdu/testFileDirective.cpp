#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/cfdp/pdu/FileDirectiveCreator.h"
#include "fsfw/cfdp/pdu/FileDirectiveReader.h"

TEST_CASE("CFDP File Directive", "[cfdp][pdu]") {
  using namespace cfdp;
  std::array<uint8_t, 32> serBuf{};
  ReturnValue_t result;
  cfdp::TransactionSeqNum seqNum = TransactionSeqNum(cfdp::WidthInBytes::ONE_BYTE, 2);
  cfdp::EntityId sourceId = EntityId(cfdp::WidthInBytes::ONE_BYTE, 0);
  cfdp::EntityId destId = EntityId(cfdp::WidthInBytes::ONE_BYTE, 1);
  PduConfig pduConf =
      PduConfig(sourceId, destId, cfdp::TransmissionMode::ACKNOWLEDGED, seqNum, false);
  uint8_t* serTarget = serBuf.data();
  const uint8_t* deserTarget = serTarget;
  size_t serSize = 0;
  auto fdSer = FileDirectiveCreator(pduConf, FileDirective::ACK, 4);

  SECTION("Serialization") {
    REQUIRE(fdSer.getSerializedSize() == 8);
    serTarget = serBuf.data();
    serSize = 0;
    result = fdSer.serialize(&serTarget, &serSize, serBuf.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    // Only version bits are set
    REQUIRE(serBuf[0] == 0b00100000);
    // PDU data field length is 5 (4 + Directive code octet)
    REQUIRE(serBuf[1] == 0);
    REQUIRE(serBuf[2] == 5);
    // Entity and Transaction Sequence number are 1 byte large
    REQUIRE(serBuf[3] == 0b00010001);
    // Source ID
    REQUIRE(serBuf[4] == 0);
    // Transaction Seq Number
    REQUIRE(serBuf[5] == 2);
    // Dest ID
    REQUIRE(serBuf[6] == 1);
    REQUIRE(serBuf[7] == FileDirective::ACK);
  }

  SECTION("Serialization fails") {
    REQUIRE(fdSer.serialize(nullptr, nullptr, 85, SerializeIF::Endianness::NETWORK) ==
            returnvalue::FAILED);
  }

  SECTION("Buffer Too Short") {
    for (uint8_t idx = 0; idx < 8; idx++) {
      serTarget = serBuf.data();
      serSize = 0;
      REQUIRE(fdSer.serialize(&serTarget, &serSize, idx, SerializeIF::Endianness::NETWORK) ==
              SerializeIF::BUFFER_TOO_SHORT);
    }
  }

  SECTION("Deserialize") {
    CHECK(fdSer.serialize(&serTarget, &serSize, serBuf.size(), SerializeIF::Endianness::NETWORK) ==
          returnvalue::OK);
    serTarget = serBuf.data();

    REQUIRE(fdSer.deSerialize(&deserTarget, &serSize, SerializeIF::Endianness::NETWORK) ==
            returnvalue::FAILED);
    deserTarget = serBuf.data();
    CHECK(serSize == 8);
    auto fdDeser = FileDirectiveReader(deserTarget, serBuf.size());
    REQUIRE(fdDeser.isNull());
    REQUIRE(not fdDeser);
    REQUIRE(fdDeser.getEndianness() == SerializeIF::Endianness::NETWORK);
    fdDeser.setEndianness(SerializeIF::Endianness::MACHINE);
    REQUIRE(fdDeser.getEndianness() == SerializeIF::Endianness::MACHINE);
    fdDeser.setEndianness(SerializeIF::Endianness::NETWORK);
    REQUIRE(fdDeser.parseData() == returnvalue::OK);
    REQUIRE(not fdDeser.isNull());
    REQUIRE(fdDeser);
    REQUIRE(fdDeser.getFileDirective() == FileDirective::ACK);
    REQUIRE(fdDeser.getPduDataFieldLen() == 5);
    REQUIRE(fdDeser.getHeaderSize() == 8);
    REQUIRE(fdDeser.getPduType() == cfdp::PduType::FILE_DIRECTIVE);

    serBuf[7] = 0xff;
    // Invalid file directive
    REQUIRE(fdDeser.parseData() == cfdp::INVALID_DIRECTIVE_FIELD);
  }
}
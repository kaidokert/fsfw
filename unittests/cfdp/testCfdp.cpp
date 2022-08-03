#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstring>

#include "fsfw/cfdp/FileSize.h"
#include "fsfw/cfdp/pdu/FileDirectiveDeserializer.h"
#include "fsfw/cfdp/pdu/FileDirectiveSerializer.h"
#include "fsfw/globalfunctions/arrayprinter.h"
#include "fsfw/serialize/SerializeAdapter.h"

TEST_CASE("CFDP Base", "[cfdp]") {
  using namespace cfdp;
  std::array<uint8_t, 32> serBuf{};
  ReturnValue_t result;
  cfdp::TransactionSeqNum seqNum = TransactionSeqNum(cfdp::WidthInBytes::ONE_BYTE, 2);
  cfdp::EntityId sourceId = EntityId(cfdp::WidthInBytes::ONE_BYTE, 0);
  cfdp::EntityId destId = EntityId(cfdp::WidthInBytes::ONE_BYTE, 1);
  PduConfig pduConf =
      PduConfig(sourceId, destId, cfdp::TransmissionModes::ACKNOWLEDGED, seqNum, false);
  uint8_t* serTarget = serBuf.data();
  const uint8_t* deserTarget = serTarget;
  size_t serSize = 0;

  SECTION("File Directive") {
    auto fdSer = FileDirectiveSerializer(pduConf, FileDirectives::ACK, 4);
    REQUIRE(fdSer.getSerializedSize() == 8);
    serTarget = serBuf.data();
    serSize = 0;
    result = fdSer.serialize(&serTarget, &serSize, serBuf.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
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
    REQUIRE(serBuf[7] == FileDirectives::ACK);

    serTarget = serBuf.data();
    size_t deserSize = 20;
    serSize = 0;
    REQUIRE(fdSer.deSerialize(&deserTarget, &deserSize, SerializeIF::Endianness::NETWORK) ==
            HasReturnvaluesIF::RETURN_FAILED);
    REQUIRE(fdSer.serialize(nullptr, nullptr, 85, SerializeIF::Endianness::NETWORK) ==
            HasReturnvaluesIF::RETURN_FAILED);
    for (uint8_t idx = 0; idx < 8; idx++) {
      serTarget = serBuf.data();
      serSize = 0;
      REQUIRE(fdSer.serialize(&serTarget, &serSize, idx, SerializeIF::Endianness::NETWORK) ==
              SerializeIF::BUFFER_TOO_SHORT);
    }

    deserTarget = serBuf.data();
    deserSize = 0;
    auto fdDeser = FileDirectiveDeserializer(deserTarget, serBuf.size());
    REQUIRE(fdDeser.getEndianness() == SerializeIF::Endianness::NETWORK);
    fdDeser.setEndianness(SerializeIF::Endianness::MACHINE);
    REQUIRE(fdDeser.getEndianness() == SerializeIF::Endianness::MACHINE);
    fdDeser.setEndianness(SerializeIF::Endianness::NETWORK);
    REQUIRE(fdDeser.parseData() == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(fdDeser.getFileDirective() == FileDirectives::ACK);
    REQUIRE(fdDeser.getPduDataFieldLen() == 5);
    REQUIRE(fdDeser.getHeaderSize() == 8);
    REQUIRE(fdDeser.getPduType() == cfdp::PduType::FILE_DIRECTIVE);

    serBuf[7] = 0xff;
    // Invalid file directive
    REQUIRE(fdDeser.parseData() == cfdp::INVALID_DIRECTIVE_FIELDS);
  }

  SECTION("File Size") {
    std::array<uint8_t, 8> fssBuf = {};
    uint8_t* buffer = fssBuf.data();
    size_t size = 0;
    cfdp::FileSize fss;
    REQUIRE(fss.getSize() == 0);
    fss.setFileSize(0x20, false);
    result = fss.serialize(&buffer, &size, fssBuf.size(), SerializeIF::Endianness::MACHINE);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    uint32_t fileSize = 0;
    result = SerializeAdapter::deSerialize(&fileSize, fssBuf.data(), nullptr,
                                           SerializeIF::Endianness::MACHINE);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(fileSize == 0x20);
  }

  SECTION("Var Length Field") {
    VarLenField defaultField;
    CHECK(defaultField.getValue() == 0);
    CHECK(defaultField.getWidth() == WidthInBytes::ONE_BYTE);
    VarLenField explicitField(WidthInBytes::FOUR_BYTES, 12);
    CHECK(explicitField.getWidth() == WidthInBytes::FOUR_BYTES);
    CHECK(explicitField.getValue() == 12);
    VarLenField fromUnsignedByteField(UnsignedByteField<uint16_t>(12));
    CHECK(fromUnsignedByteField.getWidth() == WidthInBytes::TWO_BYTES);
    CHECK(fromUnsignedByteField.getValue() == 12);
  }
}

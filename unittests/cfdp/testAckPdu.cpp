#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/cfdp/pdu/AckPduDeserializer.h"
#include "fsfw/cfdp/pdu/AckPduSerializer.h"
#include "fsfw/globalfunctions/arrayprinter.h"

TEST_CASE("ACK PDU", "[AckPdu]") {
  using namespace cfdp;
  ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
  std::array<uint8_t, 256> buf = {};
  uint8_t* bufptr = buf.data();
  size_t maxsz = buf.size();
  size_t sz = 0;
  auto seqNum = TransactionSeqNum(WidthInBytes::TWO_BYTES, 15);
  auto sourceId = EntityId(WidthInBytes::TWO_BYTES, 1);
  auto destId = EntityId(WidthInBytes::TWO_BYTES, 2);
  auto pduConf = PduConfig(TransmissionModes::ACKNOWLEDGED, seqNum, sourceId, destId);
  AckInfo ackInfo(FileDirectives::EOF_DIRECTIVE, ConditionCode::NO_ERROR,
                  AckTransactionStatus::ACTIVE);
  auto ackSerializer = AckPduSerializer(ackInfo, pduConf);
  result = ackSerializer.serialize(&bufptr, &sz, maxsz, SerializeIF::Endianness::NETWORK);
  REQUIRE(result == HasReturnvaluesIF::RETURN_OK);

  SECTION("Serialize") {
    REQUIRE(buf.data()[sz - 3] == cfdp::FileDirectives::ACK);
    REQUIRE((buf.data()[sz - 2] >> 4) == FileDirectives::EOF_DIRECTIVE);
    REQUIRE((buf.data()[sz - 2] & 0x0f) == 0);
    REQUIRE(buf.data()[sz - 1] == AckTransactionStatus::ACTIVE);
    ackInfo.setAckedDirective(FileDirectives::FINISH);
    ackInfo.setAckedConditionCode(ConditionCode::FILESTORE_REJECTION);
    ackInfo.setTransactionStatus(AckTransactionStatus::TERMINATED);
    auto ackSerializer2 = AckPduSerializer(ackInfo, pduConf);
    bufptr = buf.data();
    sz = 0;
    result = ackSerializer2.serialize(&bufptr, &sz, maxsz, SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(buf.data()[sz - 3] == cfdp::FileDirectives::ACK);
    REQUIRE((buf.data()[sz - 2] >> 4) == FileDirectives::FINISH);
    REQUIRE((buf.data()[sz - 2] & 0x0f) == 0b0001);
    REQUIRE((buf.data()[sz - 1] >> 4) == ConditionCode::FILESTORE_REJECTION);
    REQUIRE((buf.data()[sz - 1] & 0b11) == AckTransactionStatus::TERMINATED);

    bufptr = buf.data();
    sz = 0;
    ackInfo.setAckedDirective(FileDirectives::KEEP_ALIVE);
    auto ackSerializer3 = AckPduSerializer(ackInfo, pduConf);
    result = ackSerializer3.serialize(&bufptr, &sz, maxsz, SerializeIF::Endianness::NETWORK);
    // Invalid file directive
    REQUIRE(result != HasReturnvaluesIF::RETURN_OK);

    ackInfo.setAckedDirective(FileDirectives::FINISH);
    // buffer too small
    result = ackSerializer.serialize(&bufptr, &sz, 8, SerializeIF::Endianness::NETWORK);
    REQUIRE(result == SerializeIF::BUFFER_TOO_SHORT);
  }

  SECTION("Deserialize") {
    AckInfo ackInfo;
    auto reader = AckPduDeserializer(buf.data(), sz, ackInfo);
    result = reader.parseData();
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(ackInfo.getAckedDirective() == FileDirectives::EOF_DIRECTIVE);
    REQUIRE(ackInfo.getAckedConditionCode() == ConditionCode::NO_ERROR);
    REQUIRE(ackInfo.getDirectiveSubtypeCode() == 0);
    REQUIRE(ackInfo.getTransactionStatus() == AckTransactionStatus::ACTIVE);

    AckInfo newInfo = AckInfo(FileDirectives::FINISH, ConditionCode::FILESTORE_REJECTION,
                              AckTransactionStatus::TERMINATED);
    auto ackSerializer2 = AckPduSerializer(newInfo, pduConf);
    bufptr = buf.data();
    sz = 0;
    result = ackSerializer2.serialize(&bufptr, &sz, maxsz, SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);

    auto reader2 = AckPduDeserializer(buf.data(), sz, ackInfo);
    result = reader2.parseData();
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(ackInfo.getAckedDirective() == FileDirectives::FINISH);
    REQUIRE(ackInfo.getAckedConditionCode() == ConditionCode::FILESTORE_REJECTION);
    REQUIRE(ackInfo.getDirectiveSubtypeCode() == 0b0001);
    REQUIRE(ackInfo.getTransactionStatus() == AckTransactionStatus::TERMINATED);

    uint8_t prevVal = buf[sz - 2];
    buf[sz - 2] = FileDirectives::INVALID_DIRECTIVE << 4;
    result = reader2.parseData();
    REQUIRE(result == cfdp::INVALID_ACK_DIRECTIVE_FIELDS);
    buf[sz - 2] = FileDirectives::FINISH << 4 | 0b1111;
    result = reader2.parseData();
    REQUIRE(result == cfdp::INVALID_ACK_DIRECTIVE_FIELDS);
    buf[sz - 2] = prevVal;
    buf[sz - 3] = cfdp::FileDirectives::INVALID_DIRECTIVE;
    result = reader2.parseData();
    REQUIRE(result == cfdp::INVALID_DIRECTIVE_FIELDS);
    buf[sz - 3] = cfdp::FileDirectives::ACK;
    auto maxSizeTooSmall = AckPduDeserializer(buf.data(), sz - 2, ackInfo);
    result = maxSizeTooSmall.parseData();
    REQUIRE(result == SerializeIF::STREAM_TOO_SHORT);
  }
}

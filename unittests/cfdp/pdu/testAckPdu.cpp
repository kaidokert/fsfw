#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/cfdp/pdu/AckPduCreator.h"
#include "fsfw/cfdp/pdu/AckPduReader.h"
#include "fsfw/globalfunctions/arrayprinter.h"

TEST_CASE("ACK PDU", "[cfdp][pdu]") {
  using namespace cfdp;
  ReturnValue_t result;
  std::array<uint8_t, 256> buf = {};
  uint8_t* bufptr = buf.data();
  size_t maxsz = buf.size();
  size_t sz = 0;
  auto seqNum = TransactionSeqNum(WidthInBytes::TWO_BYTES, 15);
  auto sourceId = EntityId(WidthInBytes::TWO_BYTES, 1);
  auto destId = EntityId(WidthInBytes::TWO_BYTES, 2);
  auto pduConf = PduConfig(sourceId, destId, TransmissionMode::ACKNOWLEDGED, seqNum);
  AckInfo ackInfo(FileDirective::EOF_DIRECTIVE, ConditionCode::NO_ERROR,
                  AckTransactionStatus::ACTIVE);
  auto ackSerializer = AckPduCreator(ackInfo, pduConf);
  result = ackSerializer.serialize(&bufptr, &sz, maxsz, SerializeIF::Endianness::NETWORK);
  REQUIRE(result == returnvalue::OK);

  SECTION("Serialize") {
    REQUIRE(buf[sz - 3] == cfdp::FileDirective::ACK);
    REQUIRE((buf[sz - 2] >> 4) == FileDirective::EOF_DIRECTIVE);
    REQUIRE((buf[sz - 2] & 0x0f) == 0);
    REQUIRE(buf[sz - 1] == AckTransactionStatus::ACTIVE);
    ackInfo.setAckedDirective(FileDirective::FINISH);
    ackInfo.setAckedConditionCode(ConditionCode::FILESTORE_REJECTION);
    ackInfo.setTransactionStatus(AckTransactionStatus::TERMINATED);
    auto ackSerializer2 = AckPduCreator(ackInfo, pduConf);
    bufptr = buf.data();
    sz = 0;
    result = ackSerializer2.serialize(&bufptr, &sz, maxsz, SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(buf[sz - 3] == cfdp::FileDirective::ACK);
    REQUIRE((buf[sz - 2] >> 4) == FileDirective::FINISH);
    REQUIRE((buf[sz - 2] & 0x0f) == 0b0001);
    REQUIRE((buf[sz - 1] >> 4) == ConditionCode::FILESTORE_REJECTION);
    REQUIRE((buf[sz - 1] & 0b11) == AckTransactionStatus::TERMINATED);

    bufptr = buf.data();
    sz = 0;
    ackInfo.setAckedDirective(FileDirective::KEEP_ALIVE);
    auto ackSerializer3 = AckPduCreator(ackInfo, pduConf);
    result = ackSerializer3.serialize(&bufptr, &sz, maxsz, SerializeIF::Endianness::NETWORK);
    // Invalid file directive
    REQUIRE(result != returnvalue::OK);

    ackInfo.setAckedDirective(FileDirective::FINISH);
    // buffer too small
    result = ackSerializer.serialize(&bufptr, &sz, 8, SerializeIF::Endianness::NETWORK);
    REQUIRE(result == SerializeIF::BUFFER_TOO_SHORT);
  }

  SECTION("Deserialize") {
    AckInfo ackInfo2;
    auto reader = AckPduReader(buf.data(), sz, ackInfo2);
    result = reader.parseData();
    REQUIRE(result == returnvalue::OK);
    REQUIRE(ackInfo2.getAckedDirective() == FileDirective::EOF_DIRECTIVE);
    REQUIRE(ackInfo2.getAckedConditionCode() == ConditionCode::NO_ERROR);
    REQUIRE(ackInfo2.getDirectiveSubtypeCode() == 0);
    REQUIRE(ackInfo2.getTransactionStatus() == AckTransactionStatus::ACTIVE);

    AckInfo newInfo = AckInfo(FileDirective::FINISH, ConditionCode::FILESTORE_REJECTION,
                              AckTransactionStatus::TERMINATED);
    auto ackSerializer2 = AckPduCreator(newInfo, pduConf);
    bufptr = buf.data();
    sz = 0;
    result = ackSerializer2.serialize(&bufptr, &sz, maxsz, SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);

    auto reader2 = AckPduReader(buf.data(), sz, ackInfo2);
    result = reader2.parseData();
    REQUIRE(result == returnvalue::OK);
    REQUIRE(ackInfo2.getAckedDirective() == FileDirective::FINISH);
    REQUIRE(ackInfo2.getAckedConditionCode() == ConditionCode::FILESTORE_REJECTION);
    REQUIRE(ackInfo2.getDirectiveSubtypeCode() == 0b0001);
    REQUIRE(ackInfo2.getTransactionStatus() == AckTransactionStatus::TERMINATED);

    uint8_t prevVal = buf[sz - 2];
    buf[sz - 2] = FileDirective::INVALID_DIRECTIVE << 4;
    result = reader2.parseData();
    REQUIRE(result == cfdp::INVALID_ACK_DIRECTIVE_FIELDS);
    buf[sz - 2] = FileDirective::FINISH << 4 | 0b1111;
    result = reader2.parseData();
    REQUIRE(result == cfdp::INVALID_ACK_DIRECTIVE_FIELDS);
    buf[sz - 2] = prevVal;
    buf[sz - 3] = cfdp::FileDirective::INVALID_DIRECTIVE;
    result = reader2.parseData();
    REQUIRE(result == cfdp::INVALID_DIRECTIVE_FIELD);
    buf[sz - 3] = cfdp::FileDirective::ACK;
    auto maxSizeTooSmall = AckPduReader(buf.data(), sz - 2, ackInfo2);
    result = maxSizeTooSmall.parseData();
    REQUIRE(result == SerializeIF::STREAM_TOO_SHORT);
  }
}

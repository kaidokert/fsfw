#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/cfdp/pdu/PromptPduDeserializer.h"
#include "fsfw/cfdp/pdu/PromptPduSerializer.h"
#include "fsfw/globalfunctions/arrayprinter.h"

TEST_CASE("Prompt PDU", "[PromptPdu]") {
  using namespace cfdp;
  ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
  std::array<uint8_t, 256> rawBuf = {};
  uint8_t* buffer = rawBuf.data();
  size_t sz = 0;
  EntityId destId(WidthInBytes::TWO_BYTES, 2);
  TransactionSeqNum seqNum(WidthInBytes::TWO_BYTES, 15);
  EntityId sourceId(WidthInBytes::TWO_BYTES, 1);
  PduConfig pduConf(TransmissionModes::ACKNOWLEDGED, seqNum, sourceId, destId);

  SECTION("Serialize") {
    PromptPduSerializer serializer(pduConf, cfdp::PromptResponseRequired::PROMPT_KEEP_ALIVE);
    result = serializer.serialize(&buffer, &sz, rawBuf.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(serializer.getWholePduSize() == 12);
    REQUIRE(sz == 12);
    REQUIRE(serializer.getPduDataFieldLen() == 2);
    REQUIRE(rawBuf[10] == FileDirectives::PROMPT);
    REQUIRE((rawBuf[sz - 1] >> 7) & 0x01 == cfdp::PromptResponseRequired::PROMPT_KEEP_ALIVE);

    for (size_t invalidMaxSz = 0; invalidMaxSz < sz; invalidMaxSz++) {
      uint8_t* buffer = rawBuf.data();
      size_t sz = 0;
      result = serializer.serialize(&buffer, &sz, invalidMaxSz, SerializeIF::Endianness::NETWORK);
      REQUIRE(result != HasReturnvaluesIF::RETURN_OK);
    }
    for (size_t invalidSz = 1; invalidSz < sz; invalidSz++) {
      size_t locSz = invalidSz;
      uint8_t* buffer = rawBuf.data();
      result = serializer.serialize(&buffer, &locSz, sz, SerializeIF::Endianness::NETWORK);
      REQUIRE(result != HasReturnvaluesIF::RETURN_OK);
    }
  }

  SECTION("Deserialize") {
    PromptPduSerializer serializer(pduConf, cfdp::PromptResponseRequired::PROMPT_KEEP_ALIVE);
    result = serializer.serialize(&buffer, &sz, rawBuf.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);

    PromptPduDeserializer deserializer(rawBuf.data(), rawBuf.size());
    result = deserializer.parseData();
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(deserializer.getPromptResponseRequired() ==
            cfdp::PromptResponseRequired::PROMPT_KEEP_ALIVE);
    sz = deserializer.getWholePduSize();
    rawBuf[2] = 1;
    result = deserializer.parseData();
    REQUIRE(result == SerializeIF::STREAM_TOO_SHORT);
    rawBuf[2] = 2;

    for (size_t invalidMaxSz = 0; invalidMaxSz < sz; invalidMaxSz++) {
      deserializer.setData(rawBuf.data(), invalidMaxSz);
      result = deserializer.parseData();
      REQUIRE(result != HasReturnvaluesIF::RETURN_OK);
    }
  }
}

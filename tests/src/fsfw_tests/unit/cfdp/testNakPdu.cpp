#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/cfdp/pdu/NakPduDeserializer.h"
#include "fsfw/cfdp/pdu/NakPduSerializer.h"
#include "fsfw/cfdp/pdu/PduConfig.h"
#include "fsfw/globalfunctions/arrayprinter.h"

TEST_CASE("NAK PDU", "[NakPdu]") {
  using namespace cfdp;
  ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
  std::array<uint8_t, 256> nakBuffer = {};
  uint8_t* buffer = nakBuffer.data();
  size_t sz = 0;
  EntityId destId(WidthInBytes::TWO_BYTES, 2);
  TransactionSeqNum seqNum(WidthInBytes::TWO_BYTES, 15);
  EntityId sourceId(WidthInBytes::TWO_BYTES, 1);
  PduConfig pduConf(TransmissionModes::ACKNOWLEDGED, seqNum, sourceId, destId);

  FileSize startOfScope(50);
  FileSize endOfScope(1050);
  NakInfo info(startOfScope, endOfScope);
  SECTION("Serializer") {
    NakPduSerializer serializer(pduConf, info);
    result = serializer.serialize(&buffer, &sz, nakBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(serializer.getSerializedSize() == 19);
    REQUIRE(serializer.FileDirectiveSerializer::getSerializedSize() == 11);
    REQUIRE(sz == 19);
    REQUIRE(serializer.getPduDataFieldLen() == 9);
    REQUIRE(((nakBuffer[1] << 8) | nakBuffer[2]) == 0x09);
    REQUIRE(nakBuffer[10] == cfdp::FileDirectives::NAK);
    uint32_t scope = 0;
    result = SerializeAdapter::deSerialize(&scope, nakBuffer.data() + 11, nullptr,
                                           SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(scope == 50);
    result = SerializeAdapter::deSerialize(&scope, nakBuffer.data() + 15, nullptr,
                                           SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(scope == 1050);

    NakInfo::SegmentRequest segReq0(cfdp::FileSize(2020), cfdp::FileSize(2520));
    NakInfo::SegmentRequest segReq1(cfdp::FileSize(2932), cfdp::FileSize(3021));
    // Now add 2 segment requests to NAK info and serialize them as well
    std::array<NakInfo::SegmentRequest, 2> segReqs = {segReq0, segReq1};
    size_t segReqsLen = segReqs.size();
    info.setSegmentRequests(segReqs.data(), &segReqsLen, &segReqsLen);
    uint8_t* buffer = nakBuffer.data();
    size_t sz = 0;
    serializer.updateDirectiveFieldLen();
    result = serializer.serialize(&buffer, &sz, nakBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(serializer.getSerializedSize() == 35);
    REQUIRE(serializer.getPduDataFieldLen() == 25);
    REQUIRE(((nakBuffer[1] << 8) | nakBuffer[2]) == 25);
    uint32_t segReqScopes = 0;
    result = SerializeAdapter::deSerialize(&segReqScopes, nakBuffer.data() + 19, nullptr,
                                           SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(segReqScopes == 2020);
    result = SerializeAdapter::deSerialize(&segReqScopes, nakBuffer.data() + 23, nullptr,
                                           SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(segReqScopes == 2520);
    result = SerializeAdapter::deSerialize(&segReqScopes, nakBuffer.data() + 27, nullptr,
                                           SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(segReqScopes == 2932);
    result = SerializeAdapter::deSerialize(&segReqScopes, nakBuffer.data() + 31, nullptr,
                                           SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(segReqScopes == 3021);

    for (size_t maxSz = 0; maxSz < 35; maxSz++) {
      uint8_t* buffer = nakBuffer.data();
      size_t sz = 0;
      result = serializer.serialize(&buffer, &sz, maxSz, SerializeIF::Endianness::NETWORK);
      REQUIRE(result == SerializeIF::BUFFER_TOO_SHORT);
    }
    for (size_t sz = 35; sz > 0; sz--) {
      uint8_t* buffer = nakBuffer.data();
      size_t locSize = sz;
      result = serializer.serialize(&buffer, &locSize, 35, SerializeIF::Endianness::NETWORK);
      REQUIRE(result == SerializeIF::BUFFER_TOO_SHORT);
    }
  }

  SECTION("Deserializer") {
    NakPduSerializer serializer(pduConf, info);
    result = serializer.serialize(&buffer, &sz, nakBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);

    info.getStartOfScope().setFileSize(0, false);
    info.getEndOfScope().setFileSize(0, false);
    NakPduDeserializer deserializer(nakBuffer.data(), nakBuffer.size(), info);
    result = deserializer.parseData();
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    REQUIRE(deserializer.getWholePduSize() == 19);
    REQUIRE(info.getStartOfScope().getSize() == 50);
    REQUIRE(info.getEndOfScope().getSize() == 1050);

    NakInfo::SegmentRequest segReq0(cfdp::FileSize(2020), cfdp::FileSize(2520));
    NakInfo::SegmentRequest segReq1(cfdp::FileSize(2932), cfdp::FileSize(3021));
    // Now add 2 segment requests to NAK info and serialize them as well
    std::array<NakInfo::SegmentRequest, 2> segReqs = {segReq0, segReq1};
    size_t segReqsLen = segReqs.size();
    info.setSegmentRequests(segReqs.data(), &segReqsLen, &segReqsLen);
    uint8_t* buffer = nakBuffer.data();
    size_t sz = 0;
    serializer.updateDirectiveFieldLen();
    result = serializer.serialize(&buffer, &sz, nakBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);

    NakPduDeserializer deserializeWithSegReqs(nakBuffer.data(), nakBuffer.size(), info);
    result = deserializeWithSegReqs.parseData();
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    NakInfo::SegmentRequest* segReqsPtr = nullptr;
    size_t readSegReqs = 0;
    info.getSegmentRequests(&segReqsPtr, &readSegReqs, nullptr);
    REQUIRE(readSegReqs == 2);
    REQUIRE(segReqsPtr[0].first.getSize() == 2020);
    REQUIRE(segReqsPtr[0].second.getSize() == 2520);
    REQUIRE(segReqsPtr[1].first.getSize() == 2932);
    REQUIRE(segReqsPtr[1].second.getSize() == 3021);
    REQUIRE(deserializeWithSegReqs.getPduDataFieldLen() == 25);
    REQUIRE(info.getSegmentRequestsLen() == 2);
    for (size_t idx = 0; idx < 34; idx++) {
      NakPduDeserializer faultyDeserializer(nakBuffer.data(), idx, info);
      result = faultyDeserializer.parseData();
      REQUIRE(result != HasReturnvaluesIF::RETURN_OK);
    }
    for (size_t pduFieldLen = 0; pduFieldLen < 25; pduFieldLen++) {
      nakBuffer[1] = (pduFieldLen >> 8) & 0xff;
      nakBuffer[2] = pduFieldLen & 0xff;
      NakPduDeserializer faultyDeserializer(nakBuffer.data(), nakBuffer.size(), info);
      result = faultyDeserializer.parseData();
      if (pduFieldLen == 9) {
        REQUIRE(info.getSegmentRequestsLen() == 0);
      } else if (pduFieldLen == 17) {
        REQUIRE(info.getSegmentRequestsLen() == 1);
      } else if (pduFieldLen == 25) {
        REQUIRE(info.getSegmentRequestsLen() == 2);
      }
      if (pduFieldLen != 9 and pduFieldLen != 17 and pduFieldLen != 25) {
        REQUIRE(result != HasReturnvaluesIF::RETURN_OK);
      }
    }
    info.setMaxSegmentRequestLen(5);
    REQUIRE(info.getSegmentRequestsMaxLen() == 5);
  }
}

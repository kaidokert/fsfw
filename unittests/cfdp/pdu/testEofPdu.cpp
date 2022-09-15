#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/cfdp/pdu/EofPduCreator.h"
#include "fsfw/cfdp/pdu/EofPduReader.h"
#include "fsfw/globalfunctions/arrayprinter.h"

TEST_CASE("EOF PDU", "[cfdp][pdu]") {
  using namespace cfdp;

  ReturnValue_t result = returnvalue::OK;
  std::array<uint8_t, 128> buf = {};
  uint8_t* bufPtr = buf.data();
  size_t sz = 0;
  EntityId destId(WidthInBytes::TWO_BYTES, 2);
  EntityIdTlv faultLoc(destId);
  FileSize fileSize(12);
  // We can already set the fault location, it will be ignored
  EofInfo eofInfo(cfdp::ConditionCode::NO_ERROR, 5, fileSize, &faultLoc);
  TransactionSeqNum seqNum(WidthInBytes::TWO_BYTES, 15);
  EntityId sourceId(WidthInBytes::TWO_BYTES, 1);

  PduConfig pduConf(sourceId, destId, TransmissionMode::ACKNOWLEDGED, seqNum);

  auto eofSerializer = EofPduCreator(pduConf, eofInfo);
  SECTION("Serialize") {
    result = eofSerializer.serialize(&bufPtr, &sz, buf.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(((buf[1] << 8) | buf[2]) == 10);
    uint32_t checksum = 0;
    result = SerializeAdapter::deSerialize(&checksum, buf.data() + sz - 8, nullptr,
                                           SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(checksum == 5);
    uint32_t fileSizeVal = 0;
    result = SerializeAdapter::deSerialize(&fileSizeVal, buf.data() + sz - 4, nullptr,
                                           SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(fileSizeVal == 12);
    REQUIRE(buf[sz - 10] == cfdp::FileDirective::EOF_DIRECTIVE);
    REQUIRE(buf[sz - 9] == 0x00);
    REQUIRE(sz == 20);

    eofInfo.setConditionCode(cfdp::ConditionCode::FILESTORE_REJECTION);
    eofInfo.setFileSize(0x10ffffff10, true);
    pduConf.largeFile = true;
    // Should serialize with fault location now
    auto serializeWithFaultLocation = EofPduCreator(pduConf, eofInfo);
    bufPtr = buf.data();
    sz = 0;
    result = serializeWithFaultLocation.serialize(&bufPtr, &sz, buf.size(),
                                                  SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(sz == 28);
    REQUIRE(buf[10] == cfdp::FileDirective::EOF_DIRECTIVE);
    REQUIRE(buf[11] >> 4 == cfdp::ConditionCode::FILESTORE_REJECTION);
    uint64_t fileSizeLarge = 0;
    result = SerializeAdapter::deSerialize(&fileSizeLarge, buf.data() + 16, nullptr,
                                           SerializeIF::Endianness::NETWORK);
    REQUIRE(fileSizeLarge == 0x10ffffff10);
    REQUIRE(buf[sz - 4] == cfdp::TlvType::ENTITY_ID);
    // width of entity ID is 2
    REQUIRE(buf[sz - 3] == 2);
    uint16_t entityIdRaw = 0;
    result = SerializeAdapter::deSerialize(&entityIdRaw, buf.data() + sz - 2, nullptr,
                                           SerializeIF::Endianness::NETWORK);
    REQUIRE(entityIdRaw == 2);
    bufPtr = buf.data();
    sz = 0;
    for (size_t idx = 0; idx < 27; idx++) {
      result =
          serializeWithFaultLocation.serialize(&bufPtr, &sz, idx, SerializeIF::Endianness::NETWORK);
      REQUIRE(result == SerializeIF::BUFFER_TOO_SHORT);
      bufPtr = buf.data();
      sz = 0;
    }
    eofInfo.setChecksum(16);
    eofInfo.setFaultLoc(nullptr);
  }

  SECTION("Deserialize") {
    result = eofSerializer.serialize(&bufPtr, &sz, buf.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    EntityIdTlv tlv(destId);
    EofInfo emptyInfo(&tlv);
    auto deserializer = EofPduReader(buf.data(), buf.size(), emptyInfo);
    result = deserializer.parseData();
    REQUIRE(result == returnvalue::OK);
    REQUIRE(emptyInfo.getConditionCode() == cfdp::ConditionCode::NO_ERROR);
    REQUIRE(emptyInfo.getChecksum() == 5);
    REQUIRE(emptyInfo.getFileSize().getSize() == 12);

    eofInfo.setConditionCode(cfdp::ConditionCode::FILESTORE_REJECTION);
    eofInfo.setFileSize(0x10ffffff10, true);
    pduConf.largeFile = true;
    // Should serialize with fault location now
    auto serializeWithFaultLocation = EofPduCreator(pduConf, eofInfo);
    bufPtr = buf.data();
    sz = 0;
    result = serializeWithFaultLocation.serialize(&bufPtr, &sz, buf.size(),
                                                  SerializeIF::Endianness::NETWORK);
    auto deserializer2 = EofPduReader(buf.data(), buf.size(), emptyInfo);
    result = deserializer2.parseData();
    REQUIRE(result == returnvalue::OK);
    REQUIRE(emptyInfo.getConditionCode() == cfdp::ConditionCode::FILESTORE_REJECTION);
    REQUIRE(emptyInfo.getChecksum() == 5);
    REQUIRE(emptyInfo.getFileSize().getSize() == 0x10ffffff10);
    REQUIRE(emptyInfo.getFaultLoc()->getType() == cfdp::TlvType::ENTITY_ID);
    REQUIRE(emptyInfo.getFaultLoc()->getSerializedSize() == 4);
    uint16_t destId = emptyInfo.getFaultLoc()->getEntityId().getValue();
    REQUIRE(destId == 2);
    for (size_t maxSz = 0; maxSz < deserializer2.getWholePduSize() - 1; maxSz++) {
      auto invalidDeser = EofPduReader(buf.data(), maxSz, emptyInfo);
      result = invalidDeser.parseData();
      REQUIRE(result != returnvalue::OK);
    }
  }
}

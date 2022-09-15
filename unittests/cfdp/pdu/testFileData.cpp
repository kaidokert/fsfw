#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/cfdp/pdu/FileDataCreator.h"
#include "fsfw/cfdp/pdu/FileDataReader.h"
#include "fsfw/globalfunctions/arrayprinter.h"
#include "fsfw/serviceinterface.h"

TEST_CASE("File Data PDU", "[cfdp][pdu]") {
  using namespace cfdp;

  ReturnValue_t result = returnvalue::OK;
  std::array<uint8_t, 128> fileBuffer = {};
  std::array<uint8_t, 256> fileDataBuffer = {};
  uint8_t* buffer = fileDataBuffer.data();
  size_t sz = 0;
  EntityId destId(WidthInBytes::TWO_BYTES, 2);
  TransactionSeqNum seqNum(WidthInBytes::TWO_BYTES, 15);
  EntityId sourceId(WidthInBytes::TWO_BYTES, 1);
  PduConfig pduConf(sourceId, destId, TransmissionMode::ACKNOWLEDGED, seqNum);

  for (uint8_t idx = 0; idx < 10; idx++) {
    fileBuffer[idx] = idx;
  }
  FileSize offset(50);
  FileDataInfo info(offset, fileBuffer.data(), 10);

  SECTION("Serialization") {
    FileDataCreator serializer(pduConf, info);
    result =
        serializer.serialize(&buffer, &sz, fileDataBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(sz == 24);
    // 10 file bytes plus 4 byte offset
    REQUIRE(((fileDataBuffer[1] << 8) | fileDataBuffer[2]) == 14);
    // File Data -> Fourth bit is one
    REQUIRE(fileDataBuffer[0] == 0b00110000);
    uint32_t offsetRaw = 0;
    buffer = fileDataBuffer.data();
    result = SerializeAdapter::deSerialize(&offsetRaw, buffer + 10, nullptr,
                                           SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(offsetRaw == 50);
    buffer = fileDataBuffer.data() + 14;
    for (size_t idx = 0; idx < 10; idx++) {
      REQUIRE(buffer[idx] == idx);
    }

    REQUIRE(info.hasSegmentMetadata() == false);
    info.addSegmentMetadataInfo(cfdp::RecordContinuationState::CONTAINS_START_AND_END,
                                fileBuffer.data(), 10);
    REQUIRE(info.hasSegmentMetadata() == true);
    REQUIRE(info.getSegmentationControl() ==
            cfdp::SegmentationControl::NO_RECORD_BOUNDARIES_PRESERVATION);
    info.setSegmentationControl(cfdp::SegmentationControl::RECORD_BOUNDARIES_PRESERVATION);
    serializer.update();
    REQUIRE(serializer.getSegmentationControl() ==
            cfdp::SegmentationControl::RECORD_BOUNDARIES_PRESERVATION);
    buffer = fileDataBuffer.data();
    sz = 0;
    serializer.setSegmentationControl(cfdp::SegmentationControl::RECORD_BOUNDARIES_PRESERVATION);

    result =
        serializer.serialize(&buffer, &sz, fileDataBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(((fileDataBuffer[1] << 8) | fileDataBuffer[2]) == 25);
    // First bit: Seg Ctrl is set
    // Bits 1 to 3 length of enitity IDs is 2
    // Bit 4: Segment metadata flag is set
    // Bit 5 to seven: length of transaction seq num is 2
    REQUIRE(fileDataBuffer[3] == 0b10101010);
    REQUIRE((fileDataBuffer[10] >> 6) &
            0b11 == cfdp::RecordContinuationState::CONTAINS_START_AND_END);
    // Segment metadata length
    REQUIRE((fileDataBuffer[10] & 0x3f) == 10);
    buffer = fileDataBuffer.data() + 11;
    // Check segment metadata
    for (size_t idx = 0; idx < 10; idx++) {
      REQUIRE(buffer[idx] == idx);
    }
    // Check filedata
    buffer = fileDataBuffer.data() + 25;
    for (size_t idx = 0; idx < 10; idx++) {
      REQUIRE(buffer[idx] == idx);
    }

    for (size_t invalidStartSz = 1; invalidStartSz < sz; invalidStartSz++) {
      buffer = fileDataBuffer.data();
      sz = 0;
      result = serializer.serialize(&buffer, &invalidStartSz, sz, SerializeIF::Endianness::NETWORK);
      REQUIRE(result != returnvalue::OK);
    }

    info.setSegmentMetadataFlag(true);
    REQUIRE(info.getSegmentMetadataFlag() == cfdp::SegmentMetadataFlag::PRESENT);
    info.setSegmentMetadataFlag(false);
    REQUIRE(info.getSegmentMetadataFlag() == cfdp::SegmentMetadataFlag::NOT_PRESENT);
    info.setRecordContinuationState(cfdp::RecordContinuationState::CONTAINS_END_NO_START);
    info.setSegmentMetadataLen(10);
    info.setSegmentMetadata(nullptr);
    info.setFileData(nullptr, 0);
  }

  SECTION("Deserialization") {
    FileDataCreator serializer(pduConf, info);
    result =
        serializer.serialize(&buffer, &sz, fileDataBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);

    FileSize emptyOffset;
    FileDataInfo emptyInfo(emptyOffset);
    FileDataReader deserializer(fileDataBuffer.data(), fileDataBuffer.size(), emptyInfo);
    result = deserializer.parseData();
    REQUIRE(result == returnvalue::OK);
    REQUIRE(deserializer.getWholePduSize() == 24);
    REQUIRE(deserializer.getPduDataFieldLen() == 14);
    REQUIRE(deserializer.getSegmentationControl() ==
            cfdp::SegmentationControl::NO_RECORD_BOUNDARIES_PRESERVATION);
    REQUIRE(deserializer.getSegmentMetadataFlag() == cfdp::SegmentMetadataFlag::NOT_PRESENT);

    REQUIRE(emptyInfo.getOffset().getSize() == 50);
    REQUIRE(emptyInfo.hasSegmentMetadata() == false);
    size_t emptyFileSize = 0;
    const uint8_t* fileData = emptyInfo.getFileData(&emptyFileSize);
    REQUIRE(emptyFileSize == 10);
    for (size_t idx = 0; idx < 10; idx++) {
      REQUIRE(fileData[idx] == idx);
    }

    deserializer.setEndianness(SerializeIF::Endianness::NETWORK);

    info.addSegmentMetadataInfo(cfdp::RecordContinuationState::CONTAINS_START_AND_END,
                                fileBuffer.data(), 10);
    serializer.update();
    buffer = fileDataBuffer.data();
    sz = 0;
    result =
        serializer.serialize(&buffer, &sz, fileDataBuffer.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);

    result = deserializer.parseData();
    REQUIRE(result == returnvalue::OK);

    REQUIRE(emptyInfo.getOffset().getSize() == 50);
    REQUIRE(emptyInfo.hasSegmentMetadata() == true);
    REQUIRE(emptyInfo.getRecordContinuationState() ==
            cfdp::RecordContinuationState::CONTAINS_START_AND_END);
    emptyFileSize = 0;
    fileData = emptyInfo.getFileData(&emptyFileSize);
    REQUIRE(emptyFileSize == 10);
    for (size_t idx = 0; idx < 10; idx++) {
      REQUIRE(fileData[idx] == idx);
    }
    size_t segmentMetadataLen = 0;
    fileData = emptyInfo.getSegmentMetadata(&segmentMetadataLen);
    REQUIRE(segmentMetadataLen == 10);
    for (size_t idx = 0; idx < 10; idx++) {
      REQUIRE(fileData[idx] == idx);
    }
    for (size_t invalidPduField = 0; invalidPduField < 24; invalidPduField++) {
      fileDataBuffer[1] = (invalidPduField >> 8) & 0xff;
      fileDataBuffer[2] = invalidPduField & 0xff;
      result = deserializer.parseData();
      // Starting at 15, the file data is parsed. There is not leading file data length
      // field to the parser can't check whether the remaining length is valid
      if (invalidPduField < 15) {
        REQUIRE(result != returnvalue::OK);
      }
    }
  }
}

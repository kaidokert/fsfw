#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstring>

#include "CatchDefinitions.h"
#include "fsfw/cfdp/FileSize.h"
#include "fsfw/cfdp/pdu/FileDirectiveDeserializer.h"
#include "fsfw/cfdp/pdu/FileDirectiveSerializer.h"
#include "fsfw/cfdp/pdu/HeaderDeserializer.h"
#include "fsfw/cfdp/pdu/HeaderSerializer.h"
#include "fsfw/globalfunctions/arrayprinter.h"
#include "fsfw/serialize/SerializeAdapter.h"

TEST_CASE("CFDP Base", "[CfdpBase]") {
  using namespace cfdp;
  std::array<uint8_t, 32> serBuf;
  ReturnValue_t result = returnvalue::OK;
  cfdp::TransactionSeqNum seqNum = TransactionSeqNum(cfdp::WidthInBytes::ONE_BYTE, 2);
  cfdp::EntityId sourceId = EntityId(cfdp::WidthInBytes::ONE_BYTE, 0);
  cfdp::EntityId destId = EntityId(cfdp::WidthInBytes::ONE_BYTE, 1);
  PduConfig pduConf =
      PduConfig(cfdp::TransmissionModes::ACKNOWLEDGED, seqNum, sourceId, destId, false);
  uint8_t* serTarget = serBuf.data();
  const uint8_t* deserTarget = serTarget;
  size_t serSize = 0;

  SECTION("Header Serialization") {
    auto headerSerializer = HeaderSerializer(pduConf, cfdp::PduType::FILE_DIRECTIVE, 0);
    const uint8_t** dummyPtr = nullptr;
    ReturnValue_t deserResult =
        headerSerializer.deSerialize(dummyPtr, &serSize, SerializeIF::Endianness::NETWORK);
    REQUIRE(deserResult == returnvalue::FAILED);
    deserResult = headerSerializer.serialize(nullptr, &serSize, serBuf.size(),
                                             SerializeIF::Endianness::NETWORK);
    REQUIRE(deserResult == returnvalue::FAILED);
    REQUIRE(seqNum.getSerializedSize() == 1);

    REQUIRE(headerSerializer.getPduDataFieldLen() == 0);
    REQUIRE(headerSerializer.getSerializedSize() == 7);
    REQUIRE(headerSerializer.getWholePduSize() == 7);
    REQUIRE(headerSerializer.getCrcFlag() == false);
    REQUIRE(headerSerializer.getDirection() == cfdp::Direction::TOWARDS_RECEIVER);
    REQUIRE(headerSerializer.getLargeFileFlag() == false);
    REQUIRE(headerSerializer.getLenEntityIds() == 1);
    REQUIRE(headerSerializer.getLenSeqNum() == 1);
    REQUIRE(headerSerializer.getPduType() == cfdp::PduType::FILE_DIRECTIVE);
    REQUIRE(headerSerializer.getSegmentMetadataFlag() == cfdp::SegmentMetadataFlag::NOT_PRESENT);
    REQUIRE(headerSerializer.getSegmentationControl() == false);
    REQUIRE(headerSerializer.getTransmissionMode() == cfdp::TransmissionModes::ACKNOWLEDGED);

    cfdp::TransactionSeqNum seqNumLocal;
    headerSerializer.getTransactionSeqNum(seqNumLocal);
    REQUIRE(seqNumLocal.getWidth() == cfdp::WidthInBytes::ONE_BYTE);
    REQUIRE(seqNumLocal.getValue() == 2);
    cfdp::EntityId sourceDestId;
    headerSerializer.getSourceId(sourceDestId);
    REQUIRE(sourceDestId.getWidth() == cfdp::WidthInBytes::ONE_BYTE);
    REQUIRE(sourceDestId.getValue() == 0);
    headerSerializer.getDestId(sourceDestId);
    REQUIRE(sourceDestId.getWidth() == cfdp::WidthInBytes::ONE_BYTE);
    REQUIRE(sourceDestId.getValue() == 1);

    result = headerSerializer.serialize(&serTarget, &serSize, serBuf.size(),
                                        SerializeIF::Endianness::BIG);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(serSize == 7);
    // Only version bits are set
    REQUIRE(serBuf[0] == 0b00100000);
    // PDU data field length is 0
    REQUIRE(serBuf[1] == 0);
    REQUIRE(serBuf[2] == 0);
    // Entity and Transaction Sequence number are 1 byte large
    REQUIRE(serBuf[3] == 0b00010001);
    // Source ID
    REQUIRE(serBuf[4] == 0);
    // Transaction Seq Number
    REQUIRE(serBuf[5] == 2);
    // Dest ID
    REQUIRE(serBuf[6] == 1);

    for (uint8_t idx = 0; idx < 7; idx++) {
      ReturnValue_t result =
          headerSerializer.serialize(&serTarget, &serSize, idx, SerializeIF::Endianness::BIG);
      REQUIRE(result == static_cast<int>(SerializeIF::BUFFER_TOO_SHORT));
    }

    // Set PDU data field len
    headerSerializer.setPduDataFieldLen(0x0ff0);
    REQUIRE(headerSerializer.getPduDataFieldLen() == 0x0ff0);
    REQUIRE(headerSerializer.getSerializedSize() == 7);
    REQUIRE(headerSerializer.getWholePduSize() == 7 + 0x0ff0);
    serTarget = serBuf.data();
    serSize = 0;
    result = headerSerializer.serialize(&serTarget, &serSize, serBuf.size(),
                                        SerializeIF::Endianness::BIG);
    REQUIRE(serBuf[1] == 0x0f);
    REQUIRE(serBuf[2] == 0xf0);

    pduConf.crcFlag = true;
    pduConf.largeFile = true;
    pduConf.direction = cfdp::Direction::TOWARDS_SENDER;
    pduConf.mode = cfdp::TransmissionModes::UNACKNOWLEDGED;
    headerSerializer.setSegmentationControl(
        cfdp::SegmentationControl::RECORD_BOUNDARIES_PRESERVATION);
    headerSerializer.setPduType(cfdp::PduType::FILE_DATA);
    headerSerializer.setSegmentMetadataFlag(cfdp::SegmentMetadataFlag::PRESENT);
    serTarget = serBuf.data();
    serSize = 0;
    result = headerSerializer.serialize(&serTarget, &serSize, serBuf.size(),
                                        SerializeIF::Endianness::BIG);

    // Everything except version bit flipped to one now
    REQUIRE(serBuf[0] == 0x3f);
    REQUIRE(serBuf[3] == 0x99);
    pduConf.seqNum.setValue(cfdp::WidthInBytes::TWO_BYTES, 0x0fff);
    pduConf.sourceId.setValue(cfdp::WidthInBytes::FOUR_BYTES, 0xff00ff00);
    pduConf.destId.setValue(cfdp::WidthInBytes::FOUR_BYTES, 0x00ff00ff);
    REQUIRE(pduConf.sourceId.getSerializedSize() == 4);
    REQUIRE(headerSerializer.getSerializedSize() == 14);
    serTarget = serBuf.data();
    serSize = 0;
    result = headerSerializer.serialize(&serTarget, &serSize, serBuf.size(),
                                        SerializeIF::Endianness::BIG);

    for (uint8_t idx = 0; idx < 14; idx++) {
      ReturnValue_t result =
          headerSerializer.serialize(&serTarget, &serSize, idx, SerializeIF::Endianness::BIG);
      REQUIRE(result == static_cast<int>(SerializeIF::BUFFER_TOO_SHORT));
    }
    REQUIRE(headerSerializer.getCrcFlag() == true);
    REQUIRE(headerSerializer.getDirection() == cfdp::Direction::TOWARDS_SENDER);
    REQUIRE(headerSerializer.getLargeFileFlag() == true);
    REQUIRE(headerSerializer.getLenEntityIds() == 4);
    REQUIRE(headerSerializer.getLenSeqNum() == 2);
    REQUIRE(headerSerializer.getPduType() == cfdp::PduType::FILE_DATA);
    REQUIRE(headerSerializer.getSegmentMetadataFlag() == cfdp::SegmentMetadataFlag::PRESENT);
    REQUIRE(headerSerializer.getTransmissionMode() == cfdp::TransmissionModes::UNACKNOWLEDGED);
    REQUIRE(headerSerializer.getSegmentationControl() == true);
    // Last three bits are 2 now (length of seq number) and bit 1 to bit 3 is 4 (len entity IDs)
    REQUIRE(serBuf[3] == 0b11001010);
    uint32_t entityId = 0;
    size_t deSerSize = 0;
    SerializeAdapter::deSerialize(&entityId, serBuf.data() + 4, &deSerSize,
                                  SerializeIF::Endianness::NETWORK);
    REQUIRE(deSerSize == 4);
    REQUIRE(entityId == 0xff00ff00);
    uint16_t seqNum = 0;
    SerializeAdapter::deSerialize(&seqNum, serBuf.data() + 8, &deSerSize,
                                  SerializeIF::Endianness::NETWORK);
    REQUIRE(deSerSize == 2);
    REQUIRE(seqNum == 0x0fff);
    SerializeAdapter::deSerialize(&entityId, serBuf.data() + 10, &deSerSize,
                                  SerializeIF::Endianness::NETWORK);
    REQUIRE(deSerSize == 4);
    REQUIRE(entityId == 0x00ff00ff);

    result = pduConf.sourceId.setValue(cfdp::WidthInBytes::ONE_BYTE, 0xfff);
    REQUIRE(result == returnvalue::FAILED);
    result = pduConf.sourceId.setValue(cfdp::WidthInBytes::TWO_BYTES, 0xfffff);
    REQUIRE(result == returnvalue::FAILED);
    result = pduConf.sourceId.setValue(cfdp::WidthInBytes::FOUR_BYTES, 0xfffffffff);
    REQUIRE(result == returnvalue::FAILED);
    uint8_t oneByteSourceId = 32;
    serTarget = &oneByteSourceId;
    size_t deserLen = 1;
    pduConf.sourceId.deSerialize(cfdp::WidthInBytes::ONE_BYTE,
                                 const_cast<const uint8_t**>(&serTarget), &deserLen,
                                 SerializeIF::Endianness::MACHINE);
    REQUIRE(pduConf.sourceId.getValue() == 32);

    uint16_t twoByteSourceId = 0xf0f0;
    serTarget = reinterpret_cast<uint8_t*>(&twoByteSourceId);
    deserLen = 2;
    pduConf.sourceId.deSerialize(cfdp::WidthInBytes::TWO_BYTES,
                                 const_cast<const uint8_t**>(&serTarget), &deserLen,
                                 SerializeIF::Endianness::MACHINE);
    REQUIRE(pduConf.sourceId.getValue() == 0xf0f0);

    uint32_t fourByteSourceId = 0xf0f0f0f0;
    serTarget = reinterpret_cast<uint8_t*>(&fourByteSourceId);
    deserLen = 4;
    pduConf.sourceId.deSerialize(cfdp::WidthInBytes::FOUR_BYTES,
                                 const_cast<const uint8_t**>(&serTarget), &deserLen,
                                 SerializeIF::Endianness::MACHINE);
    REQUIRE(pduConf.sourceId.getValue() == 0xf0f0f0f0);

    pduConf.sourceId.setValue(cfdp::WidthInBytes::ONE_BYTE, 1);
    serTarget = serBuf.data();
    serSize = 1;
    result = pduConf.sourceId.serialize(&serTarget, &serSize, 1, SerializeIF::Endianness::MACHINE);
    REQUIRE(result == static_cast<int>(SerializeIF::BUFFER_TOO_SHORT));
  }

  SECTION("Header Deserialization") {
    // We unittested the serializer before, so we can use it now to generate valid raw  CFDP
    // data
    auto headerSerializer = HeaderSerializer(pduConf, cfdp::PduType::FILE_DIRECTIVE, 0);
    ReturnValue_t result = headerSerializer.serialize(&serTarget, &serSize, serBuf.size(),
                                                      SerializeIF::Endianness::BIG);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(serBuf[1] == 0);
    REQUIRE(serBuf[2] == 0);
    // Entity and Transaction Sequence number are 1 byte large
    REQUIRE(serBuf[3] == 0b00010001);
    REQUIRE(serSize == 7);
    // Deser call not strictly necessary
    auto headerDeser = HeaderDeserializer(serBuf.data(), serBuf.size());

    ReturnValue_t serResult = headerDeser.parseData();
    REQUIRE(serResult == returnvalue::OK);
    REQUIRE(headerDeser.getPduDataFieldLen() == 0);
    REQUIRE(headerDeser.getHeaderSize() == 7);
    REQUIRE(headerDeser.getWholePduSize() == 7);
    REQUIRE(headerDeser.getCrcFlag() == false);
    REQUIRE(headerDeser.getDirection() == cfdp::Direction::TOWARDS_RECEIVER);
    REQUIRE(headerDeser.getLargeFileFlag() == false);
    REQUIRE(headerDeser.getLenEntityIds() == 1);
    REQUIRE(headerDeser.getLenSeqNum() == 1);
    REQUIRE(headerDeser.getPduType() == cfdp::PduType::FILE_DIRECTIVE);
    REQUIRE(headerDeser.getSegmentMetadataFlag() == cfdp::SegmentMetadataFlag::NOT_PRESENT);
    REQUIRE(headerDeser.getSegmentationControl() == false);
    REQUIRE(headerDeser.getTransmissionMode() == cfdp::TransmissionModes::ACKNOWLEDGED);

    pduConf.crcFlag = true;
    pduConf.largeFile = true;
    pduConf.direction = cfdp::Direction::TOWARDS_SENDER;
    pduConf.mode = cfdp::TransmissionModes::UNACKNOWLEDGED;
    headerSerializer.setSegmentationControl(
        cfdp::SegmentationControl::RECORD_BOUNDARIES_PRESERVATION);
    headerSerializer.setPduType(cfdp::PduType::FILE_DATA);
    headerSerializer.setSegmentMetadataFlag(cfdp::SegmentMetadataFlag::PRESENT);
    result = pduConf.seqNum.setValue(cfdp::WidthInBytes::TWO_BYTES, 0x0fff);
    REQUIRE(result == returnvalue::OK);
    result = pduConf.sourceId.setValue(cfdp::WidthInBytes::FOUR_BYTES, 0xff00ff00);
    REQUIRE(result == returnvalue::OK);
    result = pduConf.destId.setValue(cfdp::WidthInBytes::FOUR_BYTES, 0x00ff00ff);
    REQUIRE(result == returnvalue::OK);
    serTarget = serBuf.data();
    serSize = 0;
    result = headerSerializer.serialize(&serTarget, &serSize, serBuf.size(),
                                        SerializeIF::Endianness::BIG);
    headerDeser = HeaderDeserializer(serBuf.data(), serBuf.size());

    result = headerDeser.parseData();
    REQUIRE(result == returnvalue::OK);
    // Everything except version bit flipped to one now
    REQUIRE(serBuf[0] == 0x3f);
    REQUIRE(serBuf[3] == 0b11001010);
    REQUIRE(headerDeser.getWholePduSize() == 14);

    REQUIRE(headerDeser.getCrcFlag() == true);
    REQUIRE(headerDeser.getDirection() == cfdp::Direction::TOWARDS_SENDER);
    REQUIRE(headerDeser.getLargeFileFlag() == true);
    REQUIRE(headerDeser.getLenEntityIds() == 4);
    REQUIRE(headerDeser.getLenSeqNum() == 2);
    REQUIRE(headerDeser.getPduType() == cfdp::PduType::FILE_DATA);
    REQUIRE(headerDeser.getSegmentMetadataFlag() == cfdp::SegmentMetadataFlag::PRESENT);
    REQUIRE(headerDeser.getSegmentationControl() == true);
    REQUIRE(headerDeser.getTransmissionMode() == cfdp::TransmissionModes::UNACKNOWLEDGED);

    cfdp::TransactionSeqNum seqNumLocal;
    headerDeser.getTransactionSeqNum(seqNumLocal);
    REQUIRE(seqNumLocal.getWidth() == cfdp::WidthInBytes::TWO_BYTES);
    REQUIRE(seqNumLocal.getValue() == 0x0fff);
    cfdp::EntityId sourceDestId;
    headerDeser.getSourceId(sourceDestId);
    REQUIRE(sourceDestId.getWidth() == cfdp::WidthInBytes::FOUR_BYTES);
    REQUIRE(sourceDestId.getValue() == 0xff00ff00);
    headerDeser.getDestId(sourceDestId);
    REQUIRE(sourceDestId.getWidth() == cfdp::WidthInBytes::FOUR_BYTES);
    REQUIRE(sourceDestId.getValue() == 0x00ff00ff);

    size_t deSerSize = headerDeser.getWholePduSize();
    serTarget = serBuf.data();
    const uint8_t** serTargetConst = const_cast<const uint8_t**>(&serTarget);
    result = headerDeser.parseData();
    REQUIRE(result == returnvalue::OK);

    headerDeser.setData(nullptr, -1);
    REQUIRE(headerDeser.getHeaderSize() == 0);
    headerDeser.setData(serBuf.data(), serBuf.size());

    serTarget = serBuf.data();
    serSize = 0;
    pduConf.sourceId.setValue(cfdp::WidthInBytes::ONE_BYTE, 22);
    pduConf.destId.setValue(cfdp::WidthInBytes::ONE_BYTE, 48);
    result = headerSerializer.serialize(&serTarget, &serSize, serBuf.size(),
                                        SerializeIF::Endianness::BIG);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(headerDeser.getWholePduSize() == 8);
    headerDeser.setData(serBuf.data(), serBuf.size());

    headerDeser.getSourceId(sourceDestId);
    REQUIRE(sourceDestId.getWidth() == cfdp::WidthInBytes::ONE_BYTE);
    REQUIRE(sourceDestId.getValue() == 22);
  }

  SECTION("File Directive") {
    auto fdSer = FileDirectiveSerializer(pduConf, FileDirectives::ACK, 4);
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
    REQUIRE(serBuf[7] == FileDirectives::ACK);

    serTarget = serBuf.data();
    size_t deserSize = 20;
    serSize = 0;
    REQUIRE(fdSer.deSerialize(&deserTarget, &deserSize, SerializeIF::Endianness::NETWORK) ==
            returnvalue::FAILED);
    REQUIRE(fdSer.serialize(nullptr, nullptr, 85, SerializeIF::Endianness::NETWORK) ==
            returnvalue::FAILED);
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
    REQUIRE(fdDeser.parseData() == returnvalue::OK);
    REQUIRE(fdDeser.getFileDirective() == FileDirectives::ACK);
    REQUIRE(fdDeser.getPduDataFieldLen() == 5);
    REQUIRE(fdDeser.getHeaderSize() == 8);
    REQUIRE(fdDeser.getPduType() == cfdp::PduType::FILE_DIRECTIVE);

    serBuf[7] = 0xff;
    // Invalid file directive
    REQUIRE(fdDeser.parseData() == cfdp::INVALID_DIRECTIVE_FIELDS);
  }

  SECTION("FileSize") {
    std::array<uint8_t, 8> fssBuf = {};
    uint8_t* buffer = fssBuf.data();
    size_t size = 0;
    cfdp::FileSize fss;
    REQUIRE(fss.getSize() == 0);
    fss.setFileSize(0x20, false);
    ReturnValue_t result =
        fss.serialize(&buffer, &size, fssBuf.size(), SerializeIF::Endianness::MACHINE);
    REQUIRE(result == returnvalue::OK);
    uint32_t fileSize = 0;
    result = SerializeAdapter::deSerialize(&fileSize, fssBuf.data(), nullptr,
                                           SerializeIF::Endianness::MACHINE);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(fileSize == 0x20);
  }
}

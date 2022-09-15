#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/cfdp/pdu/FinishedInfo.h"
#include "fsfw/cfdp/pdu/FinishedPduCreator.h"
#include "fsfw/cfdp/pdu/HeaderCreator.h"
#include "fsfw/cfdp/pdu/PduHeaderReader.h"
#include "fsfw/returnvalues/returnvalue.h"

using namespace returnvalue;

TEST_CASE("CFDP Header", "[cfdp]") {
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
  auto creator = HeaderCreator(pduConf, cfdp::PduType::FILE_DIRECTIVE, 0);

  SECTION("Header State") {
    REQUIRE(seqNum.getSerializedSize() == 1);
    REQUIRE(creator.getPduDataFieldLen() == 0);
    REQUIRE(creator.getSerializedSize() == 7);
    REQUIRE(creator.getWholePduSize() == 7);
    REQUIRE(creator.getCrcFlag() == false);
    REQUIRE(creator.getDirection() == cfdp::Direction::TOWARDS_RECEIVER);
    REQUIRE(creator.getLargeFileFlag() == false);
    REQUIRE(creator.getLenEntityIds() == 1);
    REQUIRE(creator.getLenSeqNum() == 1);
    REQUIRE(creator.getPduType() == cfdp::PduType::FILE_DIRECTIVE);
    REQUIRE(creator.getSegmentMetadataFlag() == cfdp::SegmentMetadataFlag::NOT_PRESENT);
    REQUIRE(creator.getSegmentationControl() == false);
    REQUIRE(creator.getTransmissionMode() == cfdp::TransmissionMode::ACKNOWLEDGED);
    cfdp::TransactionSeqNum seqNumLocal;
    creator.getTransactionSeqNum(seqNumLocal);
    REQUIRE(seqNumLocal.getWidth() == cfdp::WidthInBytes::ONE_BYTE);
    REQUIRE(seqNumLocal.getValue() == 2);
    cfdp::EntityId sourceDestId;
    creator.getSourceId(sourceDestId);
    REQUIRE(sourceDestId.getWidth() == cfdp::WidthInBytes::ONE_BYTE);
    REQUIRE(sourceDestId.getValue() == 0);
    creator.getDestId(sourceDestId);
    REQUIRE(sourceDestId.getWidth() == cfdp::WidthInBytes::ONE_BYTE);
    REQUIRE(sourceDestId.getValue() == 1);
  }

  SECTION("Deserialization fails") {
    const uint8_t** dummyPtr = nullptr;
    REQUIRE(creator.deSerialize(dummyPtr, &serSize, SerializeIF::Endianness::NETWORK) ==
            returnvalue::FAILED);
  }

  SECTION("Serialization fails") {
    REQUIRE(creator.serialize(nullptr, &serSize, serBuf.size(), SerializeIF::Endianness::NETWORK) ==
            returnvalue::FAILED);
  }

  SECTION("Buffer Too Short") {
    for (uint8_t idx = 0; idx < 7; idx++) {
      result = creator.serialize(&serTarget, &serSize, idx, SerializeIF::Endianness::BIG);
      REQUIRE(result == static_cast<int>(SerializeIF::BUFFER_TOO_SHORT));
    }
  }

  SECTION("Set Data Field Len") {
    // Set PDU data field len
    creator.setPduDataFieldLen(0x0ff0);
    REQUIRE(creator.getPduDataFieldLen() == 0x0ff0);
    REQUIRE(creator.getSerializedSize() == 7);
    REQUIRE(creator.getWholePduSize() == 7 + 0x0ff0);
    serTarget = serBuf.data();
    serSize = 0;
    result = creator.serialize(&serTarget, &serSize, serBuf.size(), SerializeIF::Endianness::BIG);
    REQUIRE(serBuf[1] == 0x0f);
    REQUIRE(serBuf[2] == 0xf0);
  }

  SECTION("Serialize with Fields Flipped") {
    pduConf.crcFlag = true;
    pduConf.largeFile = true;
    pduConf.direction = cfdp::Direction::TOWARDS_SENDER;
    pduConf.mode = cfdp::TransmissionMode::UNACKNOWLEDGED;
    creator.setSegmentationControl(cfdp::SegmentationControl::RECORD_BOUNDARIES_PRESERVATION);
    creator.setPduType(cfdp::PduType::FILE_DATA);
    creator.setSegmentMetadataFlag(cfdp::SegmentMetadataFlag::PRESENT);
    serTarget = serBuf.data();
    serSize = 0;

    SECTION("Regular") {
      // Everything except version bit flipped to one now
      REQUIRE(creator.serialize(&serTarget, &serSize, serBuf.size(),
                                SerializeIF::Endianness::BIG) == returnvalue::OK);
      CHECK(serBuf[0] == 0x3f);
      CHECK(serBuf[3] == 0x99);
      REQUIRE(creator.getCrcFlag() == true);
      REQUIRE(creator.getDirection() == cfdp::Direction::TOWARDS_SENDER);
      REQUIRE(creator.getLargeFileFlag() == true);
      REQUIRE(creator.getLenEntityIds() == 1);
      REQUIRE(creator.getLenSeqNum() == 1);
      REQUIRE(creator.getPduType() == cfdp::PduType::FILE_DATA);
      REQUIRE(creator.getSegmentMetadataFlag() == cfdp::SegmentMetadataFlag::PRESENT);
      REQUIRE(creator.getTransmissionMode() == cfdp::TransmissionMode::UNACKNOWLEDGED);
      REQUIRE(creator.getSegmentationControl() == true);
    }

    SECTION("Other variable sized fields") {
      pduConf.seqNum.setValue(cfdp::WidthInBytes::TWO_BYTES, 0x0fff);
      pduConf.sourceId.setValue(cfdp::WidthInBytes::FOUR_BYTES, 0xff00ff00);
      pduConf.destId.setValue(cfdp::WidthInBytes::FOUR_BYTES, 0x00ff00ff);
      REQUIRE(pduConf.sourceId.getSerializedSize() == 4);
      REQUIRE(creator.getSerializedSize() == 14);
      REQUIRE(creator.serialize(&serTarget, &serSize, serBuf.size(),
                                SerializeIF::Endianness::BIG) == returnvalue::OK);
      REQUIRE(creator.getCrcFlag() == true);
      REQUIRE(creator.getDirection() == cfdp::Direction::TOWARDS_SENDER);
      REQUIRE(creator.getLargeFileFlag() == true);
      REQUIRE(creator.getLenEntityIds() == 4);
      REQUIRE(creator.getLenSeqNum() == 2);
      REQUIRE(creator.getPduType() == cfdp::PduType::FILE_DATA);
      REQUIRE(creator.getSegmentMetadataFlag() == cfdp::SegmentMetadataFlag::PRESENT);
      REQUIRE(creator.getTransmissionMode() == cfdp::TransmissionMode::UNACKNOWLEDGED);
      REQUIRE(creator.getSegmentationControl() == true);
      // Last three bits are 2 now (length of seq number) and bit 1 to bit 3 is 4 (len entity IDs)
      REQUIRE(serBuf[3] == 0b11001010);
      uint32_t entityId = 0;
      size_t deSerSize = 0;
      SerializeAdapter::deSerialize(&entityId, serBuf.data() + 4, &deSerSize,
                                    SerializeIF::Endianness::NETWORK);
      CHECK(deSerSize == 4);
      CHECK(entityId == 0xff00ff00);
      uint16_t seqNumRaw = 0;
      SerializeAdapter::deSerialize(&seqNumRaw, serBuf.data() + 8, &deSerSize,
                                    SerializeIF::Endianness::NETWORK);
      CHECK(deSerSize == 2);
      CHECK(seqNumRaw == 0x0fff);
      SerializeAdapter::deSerialize(&entityId, serBuf.data() + 10, &deSerSize,
                                    SerializeIF::Endianness::NETWORK);
      CHECK(deSerSize == 4);
      CHECK(entityId == 0x00ff00ff);
    }

    SECTION("Buffer Too Short") {
      pduConf.seqNum.setValue(cfdp::WidthInBytes::TWO_BYTES, 0x0fff);
      pduConf.sourceId.setValue(cfdp::WidthInBytes::FOUR_BYTES, 0xff00ff00);
      pduConf.destId.setValue(cfdp::WidthInBytes::FOUR_BYTES, 0x00ff00ff);
      for (uint8_t idx = 0; idx < 14; idx++) {
        REQUIRE(creator.serialize(&serTarget, &serSize, idx, SerializeIF::Endianness::BIG) ==
                SerializeIF::BUFFER_TOO_SHORT);
      }
    }
  }

  SECTION("Invalid Variable Sized Fields") {
    result = pduConf.sourceId.setValue(cfdp::WidthInBytes::ONE_BYTE, 0xfff);
    REQUIRE(result == returnvalue::FAILED);
    result = pduConf.sourceId.setValue(cfdp::WidthInBytes::TWO_BYTES, 0xfffff);
    REQUIRE(result == returnvalue::FAILED);
    result = pduConf.sourceId.setValue(cfdp::WidthInBytes::FOUR_BYTES, 0xfffffffff);
    REQUIRE(result == returnvalue::FAILED);
  }

  SECTION("Header Serialization") {
    result = creator.serialize(&serTarget, &serSize, serBuf.size(), SerializeIF::Endianness::BIG);
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

  SECTION("Header Deserialization 0") {
    REQUIRE(creator.serialize(&serTarget, &serSize, serBuf.size(), SerializeIF::Endianness::BIG) ==
            returnvalue::OK);
    REQUIRE(serBuf[1] == 0);
    REQUIRE(serBuf[2] == 0);
    // Entity and Transaction Sequence number are 1 byte large
    REQUIRE(serBuf[3] == 0b00010001);
    REQUIRE(serSize == 7);
    // Deser call not strictly necessary
    auto reader = PduHeaderReader(serBuf.data(), serBuf.size());

    ReturnValue_t serResult = reader.parseData();
    REQUIRE(serResult == returnvalue::OK);
    REQUIRE(reader.getPduDataFieldLen() == 0);
    REQUIRE(reader.getHeaderSize() == 7);
    REQUIRE(reader.getWholePduSize() == 7);
    REQUIRE(reader.getCrcFlag() == false);
    REQUIRE(reader.getDirection() == cfdp::Direction::TOWARDS_RECEIVER);
    REQUIRE(reader.getLargeFileFlag() == false);
    REQUIRE(reader.getLenEntityIds() == 1);
    REQUIRE(reader.getLenSeqNum() == 1);
    REQUIRE(reader.getPduType() == cfdp::PduType::FILE_DIRECTIVE);
    REQUIRE(reader.getSegmentMetadataFlag() == cfdp::SegmentMetadataFlag::NOT_PRESENT);
    REQUIRE(reader.getSegmentationControl() == false);
    REQUIRE(reader.getTransmissionMode() == cfdp::TransmissionMode::ACKNOWLEDGED);
    // No PDU data contained, so the PDU data field is empty
    REQUIRE(reader.getPduDataField() == nullptr);

    size_t deSerSize = reader.getWholePduSize();
    serTarget = serBuf.data();
    const auto** serTargetConst = const_cast<const uint8_t**>(&serTarget);
    result = reader.parseData();
    REQUIRE(result == returnvalue::OK);
  }

  SECTION("Header Deserialization 1") {
    pduConf.crcFlag = true;
    pduConf.largeFile = true;
    pduConf.direction = cfdp::Direction::TOWARDS_SENDER;
    pduConf.mode = cfdp::TransmissionMode::UNACKNOWLEDGED;
    creator.setSegmentationControl(cfdp::SegmentationControl::RECORD_BOUNDARIES_PRESERVATION);
    creator.setPduType(cfdp::PduType::FILE_DATA);
    creator.setSegmentMetadataFlag(cfdp::SegmentMetadataFlag::PRESENT);
    result = pduConf.seqNum.setValue(cfdp::WidthInBytes::TWO_BYTES, 0x0fff);
    REQUIRE(result == returnvalue::OK);
    result = pduConf.sourceId.setValue(cfdp::WidthInBytes::FOUR_BYTES, 0xff00ff00);
    REQUIRE(result == returnvalue::OK);
    result = pduConf.destId.setValue(cfdp::WidthInBytes::FOUR_BYTES, 0x00ff00ff);
    REQUIRE(result == returnvalue::OK);
    serTarget = serBuf.data();
    serSize = 0;
    result = creator.serialize(&serTarget, &serSize, serBuf.size(), SerializeIF::Endianness::BIG);
    PduHeaderReader reader(serBuf.data(), serBuf.size());
    REQUIRE(reader.parseData() == returnvalue::OK);
    // Everything except version bit flipped to one now
    REQUIRE(serBuf[0] == 0x3f);
    REQUIRE(serBuf[3] == 0b11001010);
    REQUIRE(reader.getWholePduSize() == 14);

    REQUIRE(reader.getCrcFlag() == true);
    REQUIRE(reader.getDirection() == cfdp::Direction::TOWARDS_SENDER);
    REQUIRE(reader.getLargeFileFlag() == true);
    REQUIRE(reader.getLenEntityIds() == 4);
    REQUIRE(reader.getLenSeqNum() == 2);
    REQUIRE(reader.getPduType() == cfdp::PduType::FILE_DATA);
    REQUIRE(reader.getSegmentMetadataFlag() == cfdp::SegmentMetadataFlag::PRESENT);
    REQUIRE(reader.getSegmentationControl() == true);
    REQUIRE(reader.getTransmissionMode() == cfdp::TransmissionMode::UNACKNOWLEDGED);
    // Again, no data field set because this is a header only
    REQUIRE(reader.getPduDataField() == nullptr);

    cfdp::TransactionSeqNum seqNumLocal;
    reader.getTransactionSeqNum(seqNumLocal);
    REQUIRE(seqNumLocal.getWidth() == cfdp::WidthInBytes::TWO_BYTES);
    REQUIRE(seqNumLocal.getValue() == 0x0fff);
    cfdp::EntityId sourceDestId;
    reader.getSourceId(sourceDestId);
    REQUIRE(sourceDestId.getWidth() == cfdp::WidthInBytes::FOUR_BYTES);
    REQUIRE(sourceDestId.getValue() == 0xff00ff00);
    reader.getDestId(sourceDestId);
    REQUIRE(sourceDestId.getWidth() == cfdp::WidthInBytes::FOUR_BYTES);
    REQUIRE(sourceDestId.getValue() == 0x00ff00ff);
    CHECK(reader.setReadOnlyData(nullptr, -1) != returnvalue::OK);
    REQUIRE(reader.getHeaderSize() == 14);

    SECTION("Manipulate Source Dest ID") {
      serTarget = serBuf.data();
      serSize = 0;
      pduConf.sourceId.setValue(cfdp::WidthInBytes::ONE_BYTE, 22);
      pduConf.destId.setValue(cfdp::WidthInBytes::ONE_BYTE, 48);
      result = creator.serialize(&serTarget, &serSize, serBuf.size(), SerializeIF::Endianness::BIG);
      reader.getSourceId(sourceDestId);
      REQUIRE(sourceDestId.getWidth() == cfdp::WidthInBytes::ONE_BYTE);
      REQUIRE(sourceDestId.getValue() == 22);
    }
  }

  SECTION("Verify data field pointer") {
    FinishedInfo info(cfdp::ConditionCode::INACTIVITY_DETECTED,
                      cfdp::FileDeliveryCode::DATA_INCOMPLETE,
                      cfdp::FileDeliveryStatus::DISCARDED_DELIBERATELY);
    FinishPduCreator finishCreator(pduConf, info);
    REQUIRE(finishCreator.serialize(serBuf.data(), serSize, serBuf.size()) == OK);
    // This PDU contains the directive code and some finishes PDU properties packed into one byte
    // in addition to the header
    REQUIRE(finishCreator.getSerializedSize() == 9);
    PduHeaderReader reader(serBuf.data(), serBuf.size());
    REQUIRE(reader.parseData() == returnvalue::OK);
    REQUIRE(reader.getPduDataField() == serBuf.data() + 7);
  }
}

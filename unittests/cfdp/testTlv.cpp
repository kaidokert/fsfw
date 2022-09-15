#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/cfdp.h"
#include "fsfw/cfdp/VarLenFields.h"

TEST_CASE("CFDP TLV", "[cfdp][tlv]") {
  using namespace cfdp;
  ReturnValue_t result;
  std::array<uint8_t, 255> rawBuf{};
  uint8_t* serPtr = rawBuf.data();
  const uint8_t* deserPtr = rawBuf.data();
  size_t deserSize = 0;
  cfdp::EntityId sourceId = EntityId(cfdp::WidthInBytes::TWO_BYTES, 0x0ff0);

  SECTION("Entity ID Serialization") {
    REQUIRE(sourceId.serialize(&serPtr, &deserSize, rawBuf.size(),
                               SerializeIF::Endianness::NETWORK) == returnvalue::OK);
    REQUIRE(deserSize == 2);
  }

  SECTION("TLV Serialization") {
    std::array<uint8_t, 8> tlvBuf{};
    REQUIRE(sourceId.serializeBe(tlvBuf.data(), deserSize, tlvBuf.size()) == returnvalue::OK);
    auto tlv = Tlv(TlvType::ENTITY_ID, tlvBuf.data(), deserSize);
    REQUIRE(tlv.getSerializedSize() == 4);
    REQUIRE(tlv.getLengthField() == 2);
    deserSize = 0;
    REQUIRE(tlv.serialize(&serPtr, &deserSize, rawBuf.size(), SerializeIF::Endianness::NETWORK) ==
            returnvalue::OK);
    REQUIRE(deserSize == 4);
    REQUIRE(rawBuf[0] == TlvType::ENTITY_ID);
    REQUIRE(rawBuf[1] == 2);
    uint16_t entityId = 0;
    REQUIRE(SerializeAdapter::deSerialize(&entityId, rawBuf.data() + 2, &deserSize,
                                          SerializeIF::Endianness::NETWORK) == returnvalue::OK);
    REQUIRE(entityId == 0x0ff0);
  }

  SECTION("TLV Other Value") {
    auto tlv = Tlv(TlvType::ENTITY_ID, rawBuf.data(), deserSize);
    // Set new value
    sourceId.setValue(cfdp::WidthInBytes::FOUR_BYTES, 12);
    REQUIRE(sourceId.serialize(&serPtr, &deserSize, rawBuf.size(),
                               SerializeIF::Endianness::NETWORK) == returnvalue::OK);
    tlv.setValue(rawBuf.data(), cfdp::WidthInBytes::FOUR_BYTES);
    serPtr = rawBuf.data();
    deserSize = 0;
    result = tlv.serialize(&serPtr, &deserSize, rawBuf.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(rawBuf[0] == TlvType::ENTITY_ID);
    REQUIRE(rawBuf[1] == 4);

    REQUIRE(result == returnvalue::OK);
  }

  SECTION("TLV Invalid") {
    auto tlvInvalid = Tlv(cfdp::TlvType::INVALID_TLV, rawBuf.data(), 0);
    REQUIRE(tlvInvalid.serialize(&serPtr, &deserSize, rawBuf.size(),
                                 SerializeIF::Endianness::NETWORK) != returnvalue::OK);
    tlvInvalid = Tlv(cfdp::TlvType::ENTITY_ID, nullptr, 3);
    REQUIRE(tlvInvalid.serialize(&serPtr, &deserSize, rawBuf.size(),
                                 SerializeIF::Endianness::NETWORK) != returnvalue::OK);
    REQUIRE(tlvInvalid.serialize(&serPtr, &deserSize, 0, SerializeIF::Endianness::NETWORK) !=
            returnvalue::OK);
    REQUIRE(tlvInvalid.getSerializedSize() == 0);
    REQUIRE(tlvInvalid.serialize(nullptr, nullptr, 0, SerializeIF::Endianness::NETWORK) !=
            returnvalue::OK);
  }

  SECTION("TLV Zero Length Field") {
    Tlv zeroLenField(TlvType::FAULT_HANDLER, nullptr, 0);
    REQUIRE(zeroLenField.getSerializedSize() == 2);
    serPtr = rawBuf.data();
    deserSize = 0;
    REQUIRE(zeroLenField.serialize(&serPtr, &deserSize, rawBuf.size(),
                                   SerializeIF::Endianness::NETWORK) == returnvalue::OK);
    REQUIRE(rawBuf[0] == TlvType::FAULT_HANDLER);
    REQUIRE(rawBuf[1] == 0);
  }

  SECTION("TLV Deserialization") {
    // Serialization was tested before, generate raw data now
    std::array<uint8_t, 8> tlvRawBuf{};
    serPtr = tlvRawBuf.data();
    result =
        sourceId.serialize(&serPtr, &deserSize, tlvRawBuf.size(), SerializeIF::Endianness::NETWORK);
    auto tlvSerialization = Tlv(TlvType::ENTITY_ID, tlvRawBuf.data(), deserSize);
    serPtr = rawBuf.data();
    deserSize = 0;
    result = tlvSerialization.serialize(&serPtr, &deserSize, rawBuf.size(),
                                        SerializeIF::Endianness::NETWORK);
    Tlv tlv;
    deserPtr = rawBuf.data();
    result = tlv.deSerialize(&deserPtr, &deserSize, SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(tlv.getSerializedSize() == 4);
    REQUIRE(tlv.getType() == TlvType::ENTITY_ID);
    deserPtr = tlv.getValue();
    uint16_t entityId = 0;
    deserSize = 0;
    SerializeAdapter::deSerialize(&entityId, deserPtr, &deserSize,
                                  SerializeIF::Endianness::NETWORK);
    REQUIRE(entityId == 0x0ff0);

    REQUIRE(tlv.deSerialize(nullptr, nullptr, SerializeIF::Endianness::NETWORK) != returnvalue::OK);
    deserPtr = rawBuf.data();
    deserSize = 0;
    REQUIRE(tlv.deSerialize(&deserPtr, &deserSize, SerializeIF::Endianness::NETWORK) ==
            SerializeIF::STREAM_TOO_SHORT);
    // Set invalid TLV
    rawBuf[0] = TlvType::INVALID_TLV;
    deserSize = 4;
    REQUIRE(tlv.deSerialize(&deserPtr, &deserSize, SerializeIF::Endianness::NETWORK) !=
            returnvalue::OK);

    Tlv zeroLenField(TlvType::FAULT_HANDLER, nullptr, 0);
    serPtr = rawBuf.data();
    deserSize = 0;
    REQUIRE(zeroLenField.serialize(&serPtr, &deserSize, rawBuf.size(),
                                   SerializeIF::Endianness::NETWORK) == returnvalue::OK);
    deserPtr = rawBuf.data();
    result = zeroLenField.deSerialize(&deserPtr, &deserSize, SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(zeroLenField.getSerializedSize() == 2);
    REQUIRE(deserSize == 0);
  }
}

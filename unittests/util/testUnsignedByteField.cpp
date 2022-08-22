
#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/util/UnsignedByteField.h"

TEST_CASE("Unsigned Byte Field", "[unsigned-byte-field]") {
  auto testByteField = UnsignedByteField<uint32_t>(10);
  auto u32ByteField = U32ByteField(10);
  auto u16ByteField = U16ByteField(5);
  auto u8ByteField = U8ByteField(2);
  std::array<uint8_t, 16> buf{};
  size_t serLen = 0;
  SECTION("State") {
    CHECK(testByteField.getValue() == 10);
    CHECK(testByteField.getSerializedSize() == 4);
    CHECK(u32ByteField.getValue() == 10);
    CHECK(u32ByteField.getSerializedSize() == 4);
    CHECK(u16ByteField.getValue() == 5);
    CHECK(u8ByteField.getValue() == 2);
    CHECK(u8ByteField.getSerializedSize() == 1);
  }

  SECTION("Setter") {
    u32ByteField.setValue(20);
    REQUIRE(u32ByteField.getValue() == 20);
  }

  SECTION("Serialize U32") {
    CHECK(testByteField.serializeBe(buf.data(), serLen, buf.size()) == returnvalue::OK);
    CHECK(serLen == 4);
    CHECK(buf[0] == 0);
    CHECK(buf[3] == 10);
  }

  SECTION("Serialize U32 Concrete") {
    CHECK(u32ByteField.serializeBe(buf.data(), serLen, buf.size()) == returnvalue::OK);
    CHECK(serLen == 4);
    CHECK(buf[0] == 0);
    CHECK(buf[3] == 10);
  }

  SECTION("Serialize U16 Concrete") {
    CHECK(u16ByteField.serializeBe(buf.data(), serLen, buf.size()) == returnvalue::OK);
    CHECK(serLen == 2);
    CHECK(buf[0] == 0);
    CHECK(buf[1] == 5);
  }

  SECTION("Serialize U8 Concrete") {
    CHECK(u8ByteField.serializeBe(buf.data(), serLen, buf.size()) == returnvalue::OK);
    CHECK(serLen == 1);
    CHECK(buf[0] == 2);
  }

  SECTION("Deserialize") {
    buf[0] = 0x50;
    buf[1] = 0x40;
    buf[2] = 0x30;
    buf[3] = 0x20;
    size_t deserLen = 0;
    CHECK(testByteField.deSerializeBe(buf.data(), deserLen, buf.size()) == returnvalue::OK);
    CHECK(testByteField.getValue() == 0x50403020);
  }

  SECTION("Deserialize U16") {
    buf[0] = 0x50;
    buf[1] = 0x40;
    size_t deserLen = 0;
    CHECK(u16ByteField.deSerializeBe(buf.data(), deserLen, buf.size()) == returnvalue::OK);
    CHECK(u16ByteField.getValue() == 0x5040);
  }
}

#include <array>
#include <catch2/catch_test_macros.hpp>

#include "mocks/SimpleSerializable.h"

using namespace std;

TEST_CASE("Serialize IF Serialize", "[serialize-if-ser]") {
  auto simpleSer = SimpleSerializable();
  array<uint8_t, 16> buf{};
  uint8_t* ptr = buf.data();
  size_t len = 0;

  SECTION("Little Endian Normal") {
    REQUIRE(simpleSer.serialize(&ptr, &len, buf.size(), SerializeIF::Endianness::LITTLE) == HasReturnvaluesIF::RETURN_OK);
    CHECK(buf[0] == 1);
    CHECK(buf[1] == 3);
    CHECK(buf[2] == 2);
    // Verify pointer arithmetic and size increment
    CHECK(ptr == buf.data() + 3);
    CHECK(len == 3);
  }

  SECTION("Little Endian Simple") {
    REQUIRE(simpleSer.SerializeIF::serialize(buf.data(), buf.size(), SerializeIF::Endianness::LITTLE) == HasReturnvaluesIF::RETURN_OK);
    CHECK(buf[0] == 1);
    CHECK(buf[1] == 3);
    CHECK(buf[2] == 2);
  }

  SECTION("Big Endian Normal") {
    SECTION("Explicit") {
      REQUIRE(simpleSer.serialize(&ptr, &len, buf.size(), SerializeIF::Endianness::BIG) ==
              HasReturnvaluesIF::RETURN_OK);
    }
    SECTION("Network 0") {
      REQUIRE(simpleSer.serialize(&ptr, &len, buf.size(), SerializeIF::Endianness::NETWORK) == HasReturnvaluesIF::RETURN_OK);
    }
    SECTION("Network 1") {
      REQUIRE(simpleSer.serializeBe(&ptr, &len, buf.size()) == HasReturnvaluesIF::RETURN_OK);
    }

    CHECK(buf[0] == 1);
    CHECK(buf[1] == 2);
    CHECK(buf[2] == 3);
    // Verify pointer arithmetic and size increment
    CHECK(ptr == buf.data() + 3);
    CHECK(len == 3);
  }

  SECTION("Big Endian Simple") {
    SECTION("Explicit") {
      REQUIRE(simpleSer.SerializeIF::serialize(buf.data(), buf.size(), SerializeIF::Endianness::BIG) ==
              HasReturnvaluesIF::RETURN_OK);
    }
    SECTION("Network 0") {
      REQUIRE(simpleSer.SerializeIF::serialize(buf.data(), buf.size(), SerializeIF::Endianness::NETWORK) == HasReturnvaluesIF::RETURN_OK);
    }
    SECTION("Network 1") {
      REQUIRE(simpleSer.SerializeIF::serializeBe(buf.data(), buf.size()) == HasReturnvaluesIF::RETURN_OK);
    }
    CHECK(buf[0] == 1);
    CHECK(buf[1] == 2);
    CHECK(buf[2] == 3);
  }

  SECTION("Machine Endian Implicit") {
    REQUIRE(simpleSer.SerializeIF::serialize(&ptr, &len, buf.size()) ==
            HasReturnvaluesIF::RETURN_OK);
    CHECK(buf[0] == 1);
#if BYTE_ORDER_SYSTEM == LITTLE_ENDIAN
    CHECK(buf[1] == 3);
    CHECK(buf[2] == 2);
#else
    CHECK(buf[1] == 2);
    CHECK(buf[2] == 3);
#endif
    // Verify pointer arithmetic and size increment
    CHECK(ptr == buf.data() + 3);
    CHECK(len == 3);
  }

  SECTION("Machine Endian Simple Implicit") {
    REQUIRE(simpleSer.SerializeIF::serialize(buf.data(), buf.size()) ==
            HasReturnvaluesIF::RETURN_OK);
    CHECK(buf[0] == 1);
#if BYTE_ORDER_SYSTEM == LITTLE_ENDIAN
    CHECK(buf[1] == 3);
    CHECK(buf[2] == 2);
#else
    CHECK(buf[1] == 2);
    CHECK(buf[2] == 3);
#endif
  }
}

TEST_CASE("SerializeIF Deserialize", "[serialize-if-de]") {
  auto simpleSer = SimpleSerializable();
  array<uint8_t, 3> buf = {5, 0, 1};
  const uint8_t* ptr = buf.data();
  size_t len = buf.size();

  SECTION("Little Endian Normal") {
    REQUIRE(simpleSer.deSerialize(&ptr, &len, SerializeIF::Endianness::LITTLE) == HasReturnvaluesIF::RETURN_OK);
    CHECK(simpleSer.getU8() == 5);
    CHECK(simpleSer.getU16() == 0x0100);
    CHECK(ptr == buf.data() + 3);
    CHECK(len == 0);
  }

  SECTION("Little Endian Simple") {
    REQUIRE(simpleSer.SerializeIF::deSerialize(ptr, len, SerializeIF::Endianness::LITTLE) == HasReturnvaluesIF::RETURN_OK);
    CHECK(simpleSer.getU8() == 5);
    CHECK(simpleSer.getU16() == 0x0100);
  }

  SECTION("Big Endian Normal") {
    SECTION("Explicit") {
      REQUIRE(simpleSer.deSerialize(&ptr, &len, SerializeIF::Endianness::BIG) == HasReturnvaluesIF::RETURN_OK);
    }
    SECTION("Network 0") {
      REQUIRE(simpleSer.deSerialize(&ptr, &len, SerializeIF::Endianness::NETWORK) == HasReturnvaluesIF::RETURN_OK);
    }
    SECTION("Network 1") {
      REQUIRE(simpleSer.SerializeIF::deSerializeBe(&ptr, &len) == HasReturnvaluesIF::RETURN_OK);
    }
    CHECK(simpleSer.getU8() == 5);
    CHECK(simpleSer.getU16() == 1);
    CHECK(ptr == buf.data() + 3);
    CHECK(len == 0);
  }

  SECTION("Big Endian Simple") {
    SECTION("Explicit") {
      REQUIRE(simpleSer.SerializeIF::deSerialize(buf.data(), buf.size(), SerializeIF::Endianness::BIG) ==
              HasReturnvaluesIF::RETURN_OK);
    }
    SECTION("Network 0") {
      REQUIRE(simpleSer.SerializeIF::deSerialize(buf.data(), buf.size(), SerializeIF::Endianness::NETWORK) == HasReturnvaluesIF::RETURN_OK);
    }
    SECTION("Network 1") {
      REQUIRE(simpleSer.SerializeIF::deSerializeBe(buf.data(), buf.size()) == HasReturnvaluesIF::RETURN_OK);
    }
    CHECK(simpleSer.getU8() == 5);
    CHECK(simpleSer.getU16() == 1);
  }

  SECTION("Machine Endian Implicit") {
    REQUIRE(simpleSer.SerializeIF::deSerialize(&ptr, &len) == HasReturnvaluesIF::RETURN_OK);
    CHECK(simpleSer.getU8() == 5);
#if BYTE_ORDER_SYSTEM == LITTLE_ENDIAN
    CHECK(simpleSer.getU16() == 0x0100);
#else
    CHECK(simpleSer.getU16() == 1);
#endif
    // Verify pointer arithmetic and size increment
    CHECK(ptr == buf.data() + 3);
    CHECK(len == 0);
  }

  SECTION("Machine Endian Simple Implicit") {
    REQUIRE(simpleSer.SerializeIF::deSerialize(buf.data(), buf.size()) ==
            HasReturnvaluesIF::RETURN_OK);
    CHECK(simpleSer.getU8() == 5);
#if BYTE_ORDER_SYSTEM == LITTLE_ENDIAN
    CHECK(simpleSer.getU16() == 0x0100);
#else
    CHECK(simpleSer.getU16() == 1);
#endif
  }
}
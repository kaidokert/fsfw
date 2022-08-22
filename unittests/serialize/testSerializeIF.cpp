
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
    REQUIRE(simpleSer.serialize(&ptr, &len, buf.size(), SerializeIF::Endianness::LITTLE) ==
            returnvalue::OK);
    CHECK(buf[0] == 1);
    CHECK(buf[1] == 3);
    CHECK(buf[2] == 2);
    // Verify pointer arithmetic and size increment
    CHECK(ptr == buf.data() + 3);
    CHECK(len == 3);
  }

  SECTION("Little Endian Simple") {
    size_t serLen = 0xff;
    REQUIRE(simpleSer.SerializeIF::serialize(buf.data(), serLen, buf.size(),
                                             SerializeIF::Endianness::LITTLE) == returnvalue::OK);
    CHECK(buf[0] == 1);
    CHECK(buf[1] == 3);
    CHECK(buf[2] == 2);
    CHECK(serLen == 3);
  }

  SECTION("Big Endian Normal") {
    SECTION("Explicit") {
      REQUIRE(simpleSer.serialize(&ptr, &len, buf.size(), SerializeIF::Endianness::BIG) ==
              returnvalue::OK);
    }
    SECTION("Network 0") {
      REQUIRE(simpleSer.serialize(&ptr, &len, buf.size(), SerializeIF::Endianness::NETWORK) ==
              returnvalue::OK);
    }
    SECTION("Network 1") {
      REQUIRE(simpleSer.serializeBe(&ptr, &len, buf.size()) == returnvalue::OK);
    }

    CHECK(buf[0] == 1);
    CHECK(buf[1] == 2);
    CHECK(buf[2] == 3);
    // Verify pointer arithmetic and size increment
    CHECK(ptr == buf.data() + 3);
    CHECK(len == 3);
  }

  SECTION("Big Endian Simple") {
    size_t serLen = 0xff;
    SECTION("Explicit") {
      REQUIRE(simpleSer.SerializeIF::serialize(buf.data(), serLen, buf.size(),
                                               SerializeIF::Endianness::BIG) == returnvalue::OK);
    }
    SECTION("Network 0") {
      REQUIRE(simpleSer.SerializeIF::serialize(buf.data(), serLen, buf.size(),
                                               SerializeIF::Endianness::NETWORK) ==
              returnvalue::OK);
    }
    SECTION("Network 1") {
      REQUIRE(simpleSer.SerializeIF::serializeBe(buf.data(), serLen, buf.size()) ==
              returnvalue::OK);
    }
    CHECK(buf[0] == 1);
    CHECK(buf[1] == 2);
    CHECK(buf[2] == 3);
    CHECK(serLen == 3);
  }
}

TEST_CASE("SerializeIF Deserialize", "[serialize-if-de]") {
  auto simpleSer = SimpleSerializable();
  array<uint8_t, 3> buf = {5, 0, 1};
  const uint8_t* ptr = buf.data();
  size_t len = buf.size();

  SECTION("Little Endian Normal") {
    REQUIRE(simpleSer.deSerialize(&ptr, &len, SerializeIF::Endianness::LITTLE) == returnvalue::OK);
    CHECK(simpleSer.getU8() == 5);
    CHECK(simpleSer.getU16() == 0x0100);
    CHECK(ptr == buf.data() + 3);
    CHECK(len == 0);
  }

  SECTION("Little Endian Simple") {
    size_t deserLen = 0xff;
    REQUIRE(simpleSer.SerializeIF::deSerialize(ptr, deserLen, len,
                                               SerializeIF::Endianness::LITTLE) == returnvalue::OK);
    CHECK(simpleSer.getU8() == 5);
    CHECK(simpleSer.getU16() == 0x0100);
    CHECK(deserLen == 3);
  }

  SECTION("Big Endian Normal") {
    SECTION("Explicit") {
      REQUIRE(simpleSer.deSerialize(&ptr, &len, SerializeIF::Endianness::BIG) == returnvalue::OK);
    }
    SECTION("Network 0") {
      REQUIRE(simpleSer.deSerialize(&ptr, &len, SerializeIF::Endianness::NETWORK) ==
              returnvalue::OK);
    }
    SECTION("Network 1") {
      REQUIRE(simpleSer.SerializeIF::deSerializeBe(&ptr, &len) == returnvalue::OK);
    }
    CHECK(simpleSer.getU8() == 5);
    CHECK(simpleSer.getU16() == 1);
    CHECK(ptr == buf.data() + 3);
    CHECK(len == 0);
  }

  SECTION("Big Endian Simple") {
    size_t deserLen = 0xff;
    SECTION("Explicit") {
      REQUIRE(simpleSer.SerializeIF::deSerialize(buf.data(), deserLen, buf.size(),
                                                 SerializeIF::Endianness::BIG) == returnvalue::OK);
    }
    SECTION("Network 0") {
      REQUIRE(simpleSer.SerializeIF::deSerialize(buf.data(), deserLen, buf.size(),
                                                 SerializeIF::Endianness::NETWORK) ==
              returnvalue::OK);
    }
    SECTION("Network 1") {
      REQUIRE(simpleSer.SerializeIF::deSerializeBe(buf.data(), deserLen, buf.size()) ==
              returnvalue::OK);
    }
    CHECK(simpleSer.getU8() == 5);
    CHECK(simpleSer.getU16() == 1);
    CHECK(deserLen == 3);
  }
}
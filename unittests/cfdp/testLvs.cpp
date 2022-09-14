#include <array>
#include <catch2/catch_test_macros.hpp>

#include "fsfw/cfdp.h"
#include "fsfw/cfdp/VarLenFields.h"

TEST_CASE("CFDP LV", "[cfdp][lv]") {
  using namespace cfdp;
  ReturnValue_t result = returnvalue::OK;
  std::array<uint8_t, 255> rawBuf{};
  uint8_t* serPtr = rawBuf.data();
  const uint8_t* deserPtr = rawBuf.data();
  size_t deserSize = 0;
  cfdp::EntityId sourceId = EntityId(cfdp::WidthInBytes::TWO_BYTES, 0x0ff0);

  SECTION("LV Serialization") {
    std::array<uint8_t, 8> lvRawBuf{};
    serPtr = lvRawBuf.data();
    REQUIRE(sourceId.serialize(&serPtr, &deserSize, lvRawBuf.size(),
                               SerializeIF::Endianness::NETWORK) == returnvalue::OK);
    REQUIRE(deserSize == 2);

    auto lv = cfdp::Lv(lvRawBuf.data(), 2);
    REQUIRE(lv.getSerializedSize() == 3);

    SECTION("Copy") {
      auto lvCopy = cfdp::Lv(lv);
      REQUIRE(lvCopy.getSerializedSize() == 3);
      REQUIRE(lv.getValue(nullptr) == lvCopy.getValue(nullptr));
    }

    serPtr = rawBuf.data();
    deserSize = 0;
    REQUIRE(lv.serialize(&serPtr, &deserSize, rawBuf.size(), SerializeIF::Endianness::NETWORK) ==
            returnvalue::OK);
    REQUIRE(deserSize == 3);
    REQUIRE(rawBuf[0] == 2);
    uint16_t sourceIdRaw = 0;
    REQUIRE(SerializeAdapter::deSerialize(&sourceIdRaw, rawBuf.data() + 1, &deserSize,
                                          SerializeIF::Endianness::BIG) == returnvalue::OK);
    REQUIRE(sourceIdRaw == 0x0ff0);
  }

  SECTION("Empty Serialization") {
    auto lvEmpty = Lv();
    REQUIRE(lvEmpty.getSerializedSize() == 1);
    serPtr = rawBuf.data();
    deserSize = 0;
    result =
        lvEmpty.serialize(&serPtr, &deserSize, rawBuf.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(deserSize == 1);
    deserPtr = rawBuf.data();
    result = lvEmpty.deSerialize(&deserPtr, &deserSize, SerializeIF::Endianness::BIG);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(lvEmpty.getSerializedSize() == 1);
  }

  SECTION("Uninit LV") {
    std::array<uint8_t, 8> lvRawBuf{};
    serPtr = lvRawBuf.data();
    REQUIRE(sourceId.serialize(&serPtr, &deserSize, lvRawBuf.size(),
                               SerializeIF::Endianness::NETWORK) == returnvalue::OK);
    auto lv = cfdp::Lv(lvRawBuf.data(), 2);
    serPtr = rawBuf.data();
    deserSize = 0;
    result = lv.serialize(&serPtr, &deserSize, rawBuf.size(), SerializeIF::Endianness::NETWORK);
    REQUIRE(result == returnvalue::OK);
    Lv uninitLv;
    deserPtr = rawBuf.data();
    deserSize = 3;
    result = uninitLv.deSerialize(&deserPtr, &deserSize, SerializeIF::Endianness::BIG);
    REQUIRE(result == returnvalue::OK);
    REQUIRE(uninitLv.getSerializedSize() == 3);
    const uint8_t* storedValue = uninitLv.getValue(nullptr);
    uint16_t sourceIdRaw = 0;
    REQUIRE(SerializeAdapter::deSerialize(&sourceIdRaw, storedValue, &deserSize,
                                          SerializeIF::Endianness::BIG) == returnvalue::OK);
    REQUIRE(sourceIdRaw == 0x0ff0);
  }

  SECTION("Invalid Input") {
    Lv uninitLv;
    REQUIRE(uninitLv.deSerialize(nullptr, nullptr, SerializeIF::Endianness::BIG) ==
            returnvalue::FAILED);
    serPtr = rawBuf.data();
    deserSize = 0;
    REQUIRE(uninitLv.serialize(&serPtr, &deserSize, 0, SerializeIF::Endianness::BIG) ==
            SerializeIF::BUFFER_TOO_SHORT);
    REQUIRE(uninitLv.serialize(nullptr, nullptr, 12, SerializeIF::Endianness::BIG));
    deserSize = 0;
    REQUIRE(uninitLv.deSerialize(&deserPtr, &deserSize, SerializeIF::Endianness::BIG) ==
            SerializeIF::STREAM_TOO_SHORT);
  }

  SECTION("String LV String") {
    std::string filename = "hello.txt";
    StringLv sourceFileName(filename);
    REQUIRE(sourceFileName.getSerializedSize() == 1 + filename.size());
    REQUIRE(sourceFileName.serializeBe(rawBuf.data(), deserSize, rawBuf.size()) == returnvalue::OK);
    REQUIRE(rawBuf[0] == filename.size());
    std::string filenameFromRaw(reinterpret_cast<const char*>(rawBuf.data() + 1), filename.size());
    REQUIRE(filenameFromRaw == filename);
  }

  SECTION("String LV Const Char") {
    const char filename[] = "hello.txt";
    StringLv sourceFileName(filename, sizeof(filename) - 1);
    REQUIRE(sourceFileName.getSerializedSize() == 1 + sizeof(filename) - 1);
    REQUIRE(sourceFileName.serializeBe(rawBuf.data(), deserSize, rawBuf.size()) == returnvalue::OK);
    REQUIRE(rawBuf[0] == sizeof(filename) - 1);
    rawBuf[deserSize] = '\0';
    const char* filenameFromRaw = reinterpret_cast<const char*>(rawBuf.data() + 1);
    REQUIRE(std::strcmp(filename, filenameFromRaw) == 0);
  }
}

#include <fsfw/serialize/SerialBufferAdapter.h>
#include <fsfw/serialize/SerializeAdapter.h>

#include <array>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "CatchDefinitions.h"

static bool testBool = true;
static uint8_t tvUint8{5};
static uint16_t tvUint16{283};
static uint32_t tvUint32{929221};
static uint64_t tvUint64{2929329429};

static int8_t tvInt8{-16};
static int16_t tvInt16{-829};
static int32_t tvInt32{-2312};

static float tvFloat{8.2149214};
static float tvSfloat = {-922.2321321};
static double tvDouble{9.2132142141e8};
static double tvSdouble{-2.2421e19};

static std::array<uint8_t, 512> TEST_ARRAY;

TEST_CASE("Serialization size tests", "[SerSizeTest]") {
  // REQUIRE(unitTestClass.test_autoserialization() == 0);
  REQUIRE(SerializeAdapter::getSerializedSize(&testBool) == sizeof(testBool));
  REQUIRE(SerializeAdapter::getSerializedSize(&tvUint8) == sizeof(tvUint8));
  REQUIRE(SerializeAdapter::getSerializedSize(&tvUint16) == sizeof(tvUint16));
  REQUIRE(SerializeAdapter::getSerializedSize(&tvUint32) == sizeof(tvUint32));
  REQUIRE(SerializeAdapter::getSerializedSize(&tvUint64) == sizeof(tvUint64));
  REQUIRE(SerializeAdapter::getSerializedSize(&tvInt8) == sizeof(tvInt8));
  REQUIRE(SerializeAdapter::getSerializedSize(&tvInt16) == sizeof(tvInt16));
  REQUIRE(SerializeAdapter::getSerializedSize(&tvInt32) == sizeof(tvInt32));
  REQUIRE(SerializeAdapter::getSerializedSize(&tvFloat) == sizeof(tvFloat));
  REQUIRE(SerializeAdapter::getSerializedSize(&tvSfloat) == sizeof(tvSfloat));
  REQUIRE(SerializeAdapter::getSerializedSize(&tvDouble) == sizeof(tvDouble));
  REQUIRE(SerializeAdapter::getSerializedSize(&tvSdouble) == sizeof(tvSdouble));
}

TEST_CASE("Auto Serialize Adapter", "[SerAdapter]") {
  size_t serializedSize = 0;
  uint8_t* pArray = TEST_ARRAY.data();

  SECTION("SerDe") {
    size_t deserSize = 0;
    SerializeAdapter::serialize(&testBool, TEST_ARRAY.data(), &deserSize, TEST_ARRAY.size(),
                                SerializeIF::Endianness::MACHINE);
    REQUIRE(deserSize == 1);
    REQUIRE(TEST_ARRAY[0] == true);
    bool readBack = false;
    SerializeAdapter::deSerialize(&readBack, TEST_ARRAY.data(), &deserSize,
                                  SerializeIF::Endianness::MACHINE);
    REQUIRE(deserSize == 1);
    REQUIRE(readBack == true);
    SerializeAdapter::serialize(&tvUint8, TEST_ARRAY.data(), &deserSize, TEST_ARRAY.size(),
                                SerializeIF::Endianness::MACHINE);
    REQUIRE(deserSize == 1);
    REQUIRE(TEST_ARRAY[0] == 5);
    uint8_t readBackUint8 = 0;
    uint8_t* const testPtr = TEST_ARRAY.data();
    uint8_t* const shouldStayConst = testPtr;
    SerializeAdapter::deSerialize(&readBackUint8, testPtr, &deserSize,
                                  SerializeIF::Endianness::MACHINE);
    REQUIRE(testPtr == shouldStayConst);
    REQUIRE(deserSize == 1);
    REQUIRE(readBackUint8 == 5);
    SerializeAdapter::serialize(&tvUint16, TEST_ARRAY.data(), &deserSize, TEST_ARRAY.size(),
                                SerializeIF::Endianness::MACHINE);
    REQUIRE(deserSize == 2);
    deserSize = 0;
    uint16_t readBackUint16 = 0;
    SerializeAdapter::deSerialize(&readBackUint16, TEST_ARRAY.data(), &deserSize,
                                  SerializeIF::Endianness::MACHINE);
    REQUIRE(deserSize == 2);
    REQUIRE(readBackUint16 == 283);

    SerializeAdapter::serialize(&tvUint32, TEST_ARRAY.data(), &deserSize, TEST_ARRAY.size(),
                                SerializeIF::Endianness::MACHINE);
    REQUIRE(deserSize == 4);
    uint32_t readBackUint32 = 0;
    deserSize = 0;
    SerializeAdapter::deSerialize(&readBackUint32, TEST_ARRAY.data(), &deserSize,
                                  SerializeIF::Endianness::MACHINE);
    REQUIRE(deserSize == 4);
    REQUIRE(readBackUint32 == 929221);

    SerializeAdapter::serialize(&tvInt16, TEST_ARRAY.data(), &deserSize, TEST_ARRAY.size(),
                                SerializeIF::Endianness::MACHINE);
    REQUIRE(deserSize == 2);
    int16_t readBackInt16 = 0;
    SerializeAdapter::deSerialize(&readBackInt16, TEST_ARRAY.data(), &deserSize,
                                  SerializeIF::Endianness::MACHINE);
    REQUIRE(readBackInt16 == -829);
    REQUIRE(deserSize == 2);

    SerializeAdapter::serialize(&tvFloat, TEST_ARRAY.data(), &deserSize, TEST_ARRAY.size(),
                                SerializeIF::Endianness::MACHINE);
    float readBackFloat = 0.0;
    SerializeAdapter::deSerialize(&readBackFloat, TEST_ARRAY.data(), &deserSize,
                                  SerializeIF::Endianness::MACHINE);
    REQUIRE(readBackFloat == Catch::Approx(8.214921));

    SerializeAdapter::serialize(&tvSdouble, TEST_ARRAY.data(), &deserSize, TEST_ARRAY.size(),
                                SerializeIF::Endianness::MACHINE);
    double readBackSignedDouble = 0.0;
    SerializeAdapter::deSerialize(&readBackSignedDouble, TEST_ARRAY.data(), &deserSize,
                                  SerializeIF::Endianness::MACHINE);
    REQUIRE(readBackSignedDouble == Catch::Approx(-2.2421e19));

    uint8_t testBuf[4] = {1, 2, 3, 4};
    SerialBufferAdapter<uint8_t> bufferAdapter(testBuf, sizeof(testBuf));
    SerializeAdapter::serialize(&bufferAdapter, TEST_ARRAY.data(), &deserSize, TEST_ARRAY.size(),
                                SerializeIF::Endianness::MACHINE);
    REQUIRE(deserSize == 4);
    for (uint8_t idx = 0; idx < 4; idx++) {
      REQUIRE(TEST_ARRAY[idx] == idx + 1);
    }
    deserSize = 0;
    testBuf[0] = 0;
    testBuf[1] = 12;
    SerializeAdapter::deSerialize(&bufferAdapter, TEST_ARRAY.data(), &deserSize,
                                  SerializeIF::Endianness::MACHINE);
    REQUIRE(deserSize == 4);
    for (uint8_t idx = 0; idx < 4; idx++) {
      REQUIRE(testBuf[idx] == idx + 1);
    }
  }

  SECTION("Serialize incrementing") {
    SerializeAdapter::serialize(&testBool, &pArray, &serializedSize, TEST_ARRAY.size(),
                                SerializeIF::Endianness::MACHINE);
    SerializeAdapter::serialize(&tvUint8, &pArray, &serializedSize, TEST_ARRAY.size(),
                                SerializeIF::Endianness::MACHINE);
    SerializeAdapter::serialize(&tvUint16, &pArray, &serializedSize, TEST_ARRAY.size(),
                                SerializeIF::Endianness::MACHINE);
    SerializeAdapter::serialize(&tvUint32, &pArray, &serializedSize, TEST_ARRAY.size(),
                                SerializeIF::Endianness::MACHINE);
    SerializeAdapter::serialize(&tvInt8, &pArray, &serializedSize, TEST_ARRAY.size(),
                                SerializeIF::Endianness::MACHINE);
    SerializeAdapter::serialize(&tvInt16, &pArray, &serializedSize, TEST_ARRAY.size(),
                                SerializeIF::Endianness::MACHINE);
    SerializeAdapter::serialize(&tvInt32, &pArray, &serializedSize, TEST_ARRAY.size(),
                                SerializeIF::Endianness::MACHINE);
    SerializeAdapter::serialize(&tvUint64, &pArray, &serializedSize, TEST_ARRAY.size(),
                                SerializeIF::Endianness::MACHINE);
    SerializeAdapter::serialize(&tvFloat, &pArray, &serializedSize, TEST_ARRAY.size(),
                                SerializeIF::Endianness::MACHINE);
    SerializeAdapter::serialize(&tvDouble, &pArray, &serializedSize, TEST_ARRAY.size(),
                                SerializeIF::Endianness::MACHINE);
    SerializeAdapter::serialize(&tvSfloat, &pArray, &serializedSize, TEST_ARRAY.size(),
                                SerializeIF::Endianness::MACHINE);
    SerializeAdapter::serialize(&tvSdouble, &pArray, &serializedSize, TEST_ARRAY.size(),
                                SerializeIF::Endianness::MACHINE);
    REQUIRE(serializedSize == 47);
  }

  SECTION("Deserialize decrementing") {
    pArray = TEST_ARRAY.data();
    size_t remaining_size = serializedSize;
    SerializeAdapter::deSerialize(&testBool, const_cast<const uint8_t**>(&pArray), &remaining_size,
                                  SerializeIF::Endianness::MACHINE);
    SerializeAdapter::deSerialize(&tvUint8, const_cast<const uint8_t**>(&pArray), &remaining_size,
                                  SerializeIF::Endianness::MACHINE);
    SerializeAdapter::deSerialize(&tvUint16, const_cast<const uint8_t**>(&pArray), &remaining_size,
                                  SerializeIF::Endianness::MACHINE);
    SerializeAdapter::deSerialize(&tvUint32, const_cast<const uint8_t**>(&pArray), &remaining_size,
                                  SerializeIF::Endianness::MACHINE);
    SerializeAdapter::deSerialize(&tvInt8, const_cast<const uint8_t**>(&pArray), &remaining_size,
                                  SerializeIF::Endianness::MACHINE);
    SerializeAdapter::deSerialize(&tvInt16, const_cast<const uint8_t**>(&pArray), &remaining_size,
                                  SerializeIF::Endianness::MACHINE);
    SerializeAdapter::deSerialize(&tvInt32, const_cast<const uint8_t**>(&pArray), &remaining_size,
                                  SerializeIF::Endianness::MACHINE);
    SerializeAdapter::deSerialize(&tvUint64, const_cast<const uint8_t**>(&pArray), &remaining_size,
                                  SerializeIF::Endianness::MACHINE);
    SerializeAdapter::deSerialize(&tvFloat, const_cast<const uint8_t**>(&pArray), &remaining_size,
                                  SerializeIF::Endianness::MACHINE);
    SerializeAdapter::deSerialize(&tvDouble, const_cast<const uint8_t**>(&pArray), &remaining_size,
                                  SerializeIF::Endianness::MACHINE);
    SerializeAdapter::deSerialize(&tvSfloat, const_cast<const uint8_t**>(&pArray), &remaining_size,
                                  SerializeIF::Endianness::MACHINE);
    SerializeAdapter::deSerialize(&tvSdouble, const_cast<const uint8_t**>(&pArray), &remaining_size,
                                  SerializeIF::Endianness::MACHINE);

    REQUIRE(testBool == true);
    REQUIRE(tvUint8 == 5);
    REQUIRE(tvUint16 == 283);
    REQUIRE(tvUint32 == 929221);
    REQUIRE(tvUint64 == 2929329429);
    REQUIRE(tvInt8 == -16);
    REQUIRE(tvInt16 == -829);
    REQUIRE(tvInt32 == -2312);

    REQUIRE(tvFloat == Catch::Approx(8.214921));
    REQUIRE(tvDouble == Catch::Approx(9.2132142141e8));
    REQUIRE(tvSfloat == Catch::Approx(-922.2321321));
    REQUIRE(tvSdouble == Catch::Approx(-2.2421e19));
  }
}

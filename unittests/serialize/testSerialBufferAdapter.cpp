#include <fsfw/serialize/SerialBufferAdapter.h>

#include <array>
#include <catch2/catch_test_macros.hpp>

#include "CatchDefinitions.h"

static bool test_value_bool = true;
static uint16_t tv_uint16{283};
static std::array<uint8_t, 512> testArray;

TEST_CASE("Serial Buffer Adapter", "[single-file]") {
  size_t serialized_size = 0;
  test_value_bool = true;
  uint8_t* arrayPtr = testArray.data();
  std::array<uint8_t, 5> test_serial_buffer{5, 4, 3, 2, 1};
  SerialBufferAdapter<uint8_t> tv_serial_buffer_adapter =
      SerialBufferAdapter<uint8_t>(test_serial_buffer.data(), test_serial_buffer.size(), false);
  tv_uint16 = 16;

  SECTION("Serialize without size field") {
    SerializeAdapter::serialize(&test_value_bool, &arrayPtr, &serialized_size, testArray.size(),
                                SerializeIF::Endianness::MACHINE);
    SerializeAdapter::serialize(&tv_serial_buffer_adapter, &arrayPtr, &serialized_size,
                                testArray.size(), SerializeIF::Endianness::MACHINE);
    SerializeAdapter::serialize(&tv_uint16, &arrayPtr, &serialized_size, testArray.size(),
                                SerializeIF::Endianness::MACHINE);

    REQUIRE(serialized_size == 8);
    REQUIRE(testArray[0] == true);
    REQUIRE(testArray[1] == 5);
    REQUIRE(testArray[2] == 4);
    REQUIRE(testArray[3] == 3);
    REQUIRE(testArray[4] == 2);
    REQUIRE(testArray[5] == 1);
    memcpy(&tv_uint16, testArray.data() + 6, sizeof(tv_uint16));
    REQUIRE(tv_uint16 == 16);
  }

  SECTION("Serialize with size field") {
    SerialBufferAdapter<uint8_t> tv_serial_buffer_adapter_loc =
        SerialBufferAdapter<uint8_t>(test_serial_buffer.data(), test_serial_buffer.size(), true);
    serialized_size = 0;
    arrayPtr = testArray.data();
    SerializeAdapter::serialize(&test_value_bool, &arrayPtr, &serialized_size, testArray.size(),
                                SerializeIF::Endianness::MACHINE);
    SerializeAdapter::serialize(&tv_serial_buffer_adapter_loc, &arrayPtr, &serialized_size,
                                testArray.size(), SerializeIF::Endianness::MACHINE);
    SerializeAdapter::serialize(&tv_uint16, &arrayPtr, &serialized_size, testArray.size(),
                                SerializeIF::Endianness::MACHINE);

    REQUIRE(serialized_size == 9);
    REQUIRE(testArray[0] == true);
    REQUIRE(testArray[1] == 5);
    REQUIRE(testArray[2] == 5);
    REQUIRE(testArray[3] == 4);
    REQUIRE(testArray[4] == 3);
    REQUIRE(testArray[5] == 2);
    REQUIRE(testArray[6] == 1);
    memcpy(&tv_uint16, testArray.data() + 7, sizeof(tv_uint16));
    REQUIRE(tv_uint16 == 16);
  }

  SECTION("Test set buffer function") {
    SerialBufferAdapter<uint8_t> tv_serial_buffer_adapter_loc =
        SerialBufferAdapter<uint8_t>((uint8_t*)nullptr, 0, true);
    tv_serial_buffer_adapter_loc.setConstBuffer(test_serial_buffer.data(),
                                                test_serial_buffer.size());
    serialized_size = 0;
    arrayPtr = testArray.data();
    SerializeAdapter::serialize(&test_value_bool, &arrayPtr, &serialized_size, testArray.size(),
                                SerializeIF::Endianness::MACHINE);
    SerializeAdapter::serialize(&tv_serial_buffer_adapter_loc, &arrayPtr, &serialized_size,
                                testArray.size(), SerializeIF::Endianness::MACHINE);
    SerializeAdapter::serialize(&tv_uint16, &arrayPtr, &serialized_size, testArray.size(),
                                SerializeIF::Endianness::MACHINE);
    REQUIRE(serialized_size == 9);
    REQUIRE(testArray[0] == true);
    REQUIRE(testArray[1] == 5);
    REQUIRE(testArray[2] == 5);
    REQUIRE(testArray[3] == 4);
    REQUIRE(testArray[4] == 3);
    REQUIRE(testArray[5] == 2);
    REQUIRE(testArray[6] == 1);
    memcpy(&tv_uint16, testArray.data() + 7, sizeof(tv_uint16));
    REQUIRE(tv_uint16 == 16);
  }

  SECTION("Deserialization with size field") {
    size_t buffer_size = 4;
    memcpy(testArray.data(), &buffer_size, sizeof(uint16_t));
    testArray[2] = 1;
    testArray[3] = 1;
    testArray[4] = 1;
    testArray[5] = 0;
    std::array<uint8_t, 4> test_recv_array{};
    arrayPtr = testArray.data();
    // copy testArray[1] to testArray[4] into receive buffer, skip
    // size field (testArray[0]) for deSerialization.
    SerialBufferAdapter<uint16_t> tv_serial_buffer_adapter3 =
        SerialBufferAdapter<uint16_t>(test_recv_array.data(), 4, true);
    // Deserialization
    size_t size = 6;
    auto result = tv_serial_buffer_adapter3.deSerialize(const_cast<const uint8_t**>(&arrayPtr),
                                                        &size, SerializeIF::Endianness::MACHINE);
    REQUIRE(result == returnvalue::OK);
    CHECK(test_recv_array[0] == 1);
    CHECK(test_recv_array[1] == 1);
    CHECK(test_recv_array[2] == 1);
    CHECK(test_recv_array[3] == 0);
  }

  SECTION("Deserialization without size field") {
    size_t buffer_size = 4;
    memcpy(testArray.data(), &buffer_size, sizeof(uint16_t));
    testArray[2] = 1;
    testArray[3] = 1;
    testArray[4] = 1;
    testArray[5] = 0;
    std::array<uint8_t, 4> test_recv_array{};
    arrayPtr = testArray.data() + 2;
    // copy testArray[1] to testArray[4] into receive buffer, skip
    // size field (testArray[0])
    SerialBufferAdapter<uint16_t> tv_serial_buffer_adapter3 =
        SerialBufferAdapter<uint16_t>(test_recv_array.data(), 4, false);
    // Deserialization
    size_t size = 4;
    tv_serial_buffer_adapter3.deSerialize(const_cast<const uint8_t**>(&arrayPtr), &size,
                                          SerializeIF::Endianness::MACHINE);
    CHECK(test_recv_array[0] == 1);
    CHECK(test_recv_array[1] == 1);
    CHECK(test_recv_array[2] == 1);
    CHECK(test_recv_array[3] == 0);
  }
}

#include "fsfw_tests/internal/serialize/IntTestSerialization.h"

#include <fsfw/serialize/SerialBufferAdapter.h>
#include <fsfw/serialize/SerializeElement.h>
#include <fsfw/serialize/SerializeIF.h>

#include <array>

#include "fsfw_tests/internal/UnittDefinitions.h"

std::array<uint8_t, 512> testserialize::test_array = {0};

ReturnValue_t testserialize::test_serialization() {
  // Here, we test all serialization tools. First test basic cases.
  ReturnValue_t result = test_endianness_tools();
  if (result != returnvalue::OK) {
    return result;
  }
  result = test_autoserialization();
  if (result != returnvalue::OK) {
    return result;
  }
  result = test_serial_buffer_adapter();
  if (result != returnvalue::OK) {
    return result;
  }
  return returnvalue::OK;
}

ReturnValue_t testserialize::test_endianness_tools() {
  std::string id = "[test_endianness_tools]";
  test_array[0] = 0;
  test_array[1] = 0;
  uint16_t two_byte_value = 1;
  size_t size = 0;
  uint8_t* p_array = test_array.data();
  SerializeAdapter::serialize(&two_byte_value, &p_array, &size, 2,
                              SerializeIF::Endianness::MACHINE);
  // Little endian: Value one on first byte
  if (test_array[0] != 1 and test_array[1] != 0) {
    return unitt::put_error(id);
  }

  p_array = test_array.data();
  size = 0;
  SerializeAdapter::serialize(&two_byte_value, &p_array, &size, 2, SerializeIF::Endianness::BIG);
  // Big endian: Value one on second byte
  if (test_array[0] != 0 and test_array[1] != 1) {
    return unitt::put_error(id);
  }
  return returnvalue::OK;
}

ReturnValue_t testserialize::test_autoserialization() {
  std::string id = "[test_autoserialization]";
  // Unit Test getSerializedSize
  if (SerializeAdapter::getSerializedSize(&tv::tv_bool) != sizeof(tv::tv_bool) or
      SerializeAdapter::getSerializedSize(&tv::tv_uint8) != sizeof(tv::tv_uint8) or
      SerializeAdapter::getSerializedSize(&tv::tv_uint16) != sizeof(tv::tv_uint16) or
      SerializeAdapter::getSerializedSize(&tv::tv_uint32) != sizeof(tv::tv_uint32) or
      SerializeAdapter::getSerializedSize(&tv::tv_uint64) != sizeof(tv::tv_uint64) or
      SerializeAdapter::getSerializedSize(&tv::tv_int8) != sizeof(tv::tv_int8) or
      SerializeAdapter::getSerializedSize(&tv::tv_double) != sizeof(tv::tv_double) or
      SerializeAdapter::getSerializedSize(&tv::tv_int16) != sizeof(tv::tv_int16) or
      SerializeAdapter::getSerializedSize(&tv::tv_int32) != sizeof(tv::tv_int32) or
      SerializeAdapter::getSerializedSize(&tv::tv_float) != sizeof(tv::tv_float)) {
    return unitt::put_error(id);
  }

  size_t serialized_size = 0;
  uint8_t* p_array = test_array.data();

  SerializeAdapter::serialize(&tv::tv_bool, &p_array, &serialized_size, test_array.size(),
                              SerializeIF::Endianness::MACHINE);
  SerializeAdapter::serialize(&tv::tv_uint8, &p_array, &serialized_size, test_array.size(),
                              SerializeIF::Endianness::MACHINE);
  SerializeAdapter::serialize(&tv::tv_uint16, &p_array, &serialized_size, test_array.size(),
                              SerializeIF::Endianness::MACHINE);
  SerializeAdapter::serialize(&tv::tv_uint32, &p_array, &serialized_size, test_array.size(),
                              SerializeIF::Endianness::MACHINE);
  SerializeAdapter::serialize(&tv::tv_int8, &p_array, &serialized_size, test_array.size(),
                              SerializeIF::Endianness::MACHINE);
  SerializeAdapter::serialize(&tv::tv_int16, &p_array, &serialized_size, test_array.size(),
                              SerializeIF::Endianness::MACHINE);
  SerializeAdapter::serialize(&tv::tv_int32, &p_array, &serialized_size, test_array.size(),
                              SerializeIF::Endianness::MACHINE);
  SerializeAdapter::serialize(&tv::tv_uint64, &p_array, &serialized_size, test_array.size(),
                              SerializeIF::Endianness::MACHINE);
  SerializeAdapter::serialize(&tv::tv_float, &p_array, &serialized_size, test_array.size(),
                              SerializeIF::Endianness::MACHINE);
  SerializeAdapter::serialize(&tv::tv_double, &p_array, &serialized_size, test_array.size(),
                              SerializeIF::Endianness::MACHINE);
  SerializeAdapter::serialize(&tv::tv_sfloat, &p_array, &serialized_size, test_array.size(),
                              SerializeIF::Endianness::MACHINE);
  SerializeAdapter::serialize(&tv::tv_sdouble, &p_array, &serialized_size, test_array.size(),
                              SerializeIF::Endianness::MACHINE);
  // expected size is 1 + 1 + 2 + 4 + 1 + 2 + 4 + 8 + 4 + 8 + 4 + 8
  if (serialized_size != 47) {
    return unitt::put_error(id);
  }

  p_array = test_array.data();
  size_t remaining_size = serialized_size;
  bool tv_bool;
  uint8_t tv_uint8;
  uint16_t tv_uint16;
  uint32_t tv_uint32;
  int8_t tv_int8;
  int16_t tv_int16;
  int32_t tv_int32;
  uint64_t tv_uint64;
  float tv_float;
  double tv_double;
  float tv_sfloat;
  double tv_sdouble;

  SerializeAdapter::deSerialize(&tv_bool, const_cast<const uint8_t**>(&p_array), &remaining_size,
                                SerializeIF::Endianness::MACHINE);
  SerializeAdapter::deSerialize(&tv_uint8, const_cast<const uint8_t**>(&p_array), &remaining_size,
                                SerializeIF::Endianness::MACHINE);
  SerializeAdapter::deSerialize(&tv_uint16, const_cast<const uint8_t**>(&p_array), &remaining_size,
                                SerializeIF::Endianness::MACHINE);
  SerializeAdapter::deSerialize(&tv_uint32, const_cast<const uint8_t**>(&p_array), &remaining_size,
                                SerializeIF::Endianness::MACHINE);
  SerializeAdapter::deSerialize(&tv_int8, const_cast<const uint8_t**>(&p_array), &remaining_size,
                                SerializeIF::Endianness::MACHINE);
  SerializeAdapter::deSerialize(&tv_int16, const_cast<const uint8_t**>(&p_array), &remaining_size,
                                SerializeIF::Endianness::MACHINE);
  SerializeAdapter::deSerialize(&tv_int32, const_cast<const uint8_t**>(&p_array), &remaining_size,
                                SerializeIF::Endianness::MACHINE);
  SerializeAdapter::deSerialize(&tv_uint64, const_cast<const uint8_t**>(&p_array), &remaining_size,
                                SerializeIF::Endianness::MACHINE);
  SerializeAdapter::deSerialize(&tv_float, const_cast<const uint8_t**>(&p_array), &remaining_size,
                                SerializeIF::Endianness::MACHINE);
  SerializeAdapter::deSerialize(&tv_double, const_cast<const uint8_t**>(&p_array), &remaining_size,
                                SerializeIF::Endianness::MACHINE);
  SerializeAdapter::deSerialize(&tv_sfloat, const_cast<const uint8_t**>(&p_array), &remaining_size,
                                SerializeIF::Endianness::MACHINE);
  SerializeAdapter::deSerialize(&tv_sdouble, const_cast<const uint8_t**>(&p_array), &remaining_size,
                                SerializeIF::Endianness::MACHINE);

  if (tv_bool != tv::tv_bool or tv_uint8 != tv::tv_uint8 or tv_uint16 != tv::tv_uint16 or
      tv_uint32 != tv::tv_uint32 or tv_uint64 != tv::tv_uint64 or tv_int8 != tv::tv_int8 or
      tv_int16 != tv::tv_int16 or tv_int32 != tv::tv_int32) {
    return unitt::put_error(id);
  }

  // These epsilon values were just guessed.. It appears to work though.
  if (abs(tv_float - tv::tv_float) > 0.0001 or abs(tv_double - tv::tv_double) > 0.01 or
      abs(tv_sfloat - tv::tv_sfloat) > 0.0001 or abs(tv_sdouble - tv::tv_sdouble) > 0.01) {
    return unitt::put_error(id);
  }

  // Check overflow
  return returnvalue::OK;
}

// TODO: Also test for constant buffers.
ReturnValue_t testserialize::test_serial_buffer_adapter() {
  std::string id = "[test_serial_buffer_adapter]";

  // I will skip endian swapper testing, its going to be changed anyway..
  // uint8_t tv::tv_uint8_swapped = EndianSwapper::swap(tv::tv_uint8);

  size_t serialized_size = 0;
  uint8_t* p_array = test_array.data();
  std::array<uint8_t, 5> test_serial_buffer{5, 4, 3, 2, 1};
  SerialBufferAdapter<uint8_t> tv_serial_buffer_adapter =
      SerialBufferAdapter<uint8_t>(test_serial_buffer.data(), test_serial_buffer.size(), false);
  uint16_t testUint16 = 16;

  SerializeAdapter::serialize(&tv::tv_bool, &p_array, &serialized_size, test_array.size(),
                              SerializeIF::Endianness::MACHINE);
  SerializeAdapter::serialize(&tv_serial_buffer_adapter, &p_array, &serialized_size,
                              test_array.size(), SerializeIF::Endianness::MACHINE);
  SerializeAdapter::serialize(&testUint16, &p_array, &serialized_size, test_array.size(),
                              SerializeIF::Endianness::MACHINE);

  if (serialized_size != 8 or test_array[0] != true or test_array[1] != 5 or test_array[2] != 4 or
      test_array[3] != 3 or test_array[4] != 2 or test_array[5] != 1) {
    return unitt::put_error(id);
  }
  memcpy(&testUint16, test_array.data() + 6, sizeof(testUint16));
  if (testUint16 != 16) {
    return unitt::put_error(id);
  }

  // Serialize with size field
  SerialBufferAdapter<uint8_t> tv_serial_buffer_adapter2 =
      SerialBufferAdapter<uint8_t>(test_serial_buffer.data(), test_serial_buffer.size(), true);
  serialized_size = 0;
  p_array = test_array.data();
  SerializeAdapter::serialize(&tv::tv_bool, &p_array, &serialized_size, test_array.size(),
                              SerializeIF::Endianness::MACHINE);
  SerializeAdapter::serialize(&tv_serial_buffer_adapter2, &p_array, &serialized_size,
                              test_array.size(), SerializeIF::Endianness::MACHINE);
  SerializeAdapter::serialize(&testUint16, &p_array, &serialized_size, test_array.size(),
                              SerializeIF::Endianness::MACHINE);

  if (serialized_size != 9 or test_array[0] != true or test_array[1] != 5 or test_array[2] != 5 or
      test_array[3] != 4 or test_array[4] != 3 or test_array[5] != 2 or test_array[6] != 1) {
    return unitt::put_error(id);
  }
  memcpy(&testUint16, test_array.data() + 7, sizeof(testUint16));
  if (testUint16 != 16) {
    return unitt::put_error(id);
  }
  return returnvalue::OK;
}

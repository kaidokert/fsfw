#ifndef FSFW_UNITTEST_INTERNAL_INTTESTSERIALIZATION_H_
#define FSFW_UNITTEST_INTERNAL_INTTESTSERIALIZATION_H_

#include "../../returnvalues/HasReturnvaluesIF.h"
#include <array>

namespace testserialize {
ReturnValue_t test_serialization();
ReturnValue_t test_endianness_tools();
ReturnValue_t test_autoserialization();
ReturnValue_t test_serial_buffer_adapter();

extern std::array<uint8_t, 512> test_array;
}

#endif /* FSFW_UNITTEST_INTERNAL_INTTESTSERIALIZATION_H_ */

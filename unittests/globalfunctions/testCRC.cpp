#include <array>

#include "CatchDefinitions.h"
#include "catch2/catch_test_macros.hpp"
#include "fsfw/globalfunctions/CRC.h"

TEST_CASE("CRC", "[CRC]") {
  std::array<uint8_t, 10> testData = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  uint16_t crc = CRC::crc16ccitt(testData.data(), 10);
  REQUIRE(crc == 49729);
  for (uint8_t index = 0; index < testData.size(); index++) {
    REQUIRE(testData[index] == index);
  }
}
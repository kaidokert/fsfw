#include <catch2/catch_test_macros.hpp>

#include "fsfw/globalfunctions/bitutility.h"

TEST_CASE("Bitutility", "[Bitutility]") {
  uint8_t dummyByte = 0;
  bool bitSet = false;
  for (uint8_t pos = 0; pos < 8; pos++) {
    bitutil::set(&dummyByte, pos);
    REQUIRE(dummyByte == (1 << (7 - pos)));
    bitutil::get(&dummyByte, pos, bitSet);
    REQUIRE(bitSet == 1);
    dummyByte = 0;
  }

  dummyByte = 0xff;
  for (uint8_t pos = 0; pos < 8; pos++) {
    bitutil::get(&dummyByte, pos, bitSet);
    REQUIRE(bitSet == 1);
    bitutil::clear(&dummyByte, pos);
    bitutil::get(&dummyByte, pos, bitSet);
    REQUIRE(bitSet == 0);
    dummyByte = 0xff;
  }

  dummyByte = 0xf0;
  for (uint8_t pos = 0; pos < 8; pos++) {
    if (pos < 4) {
      bitutil::get(&dummyByte, pos, bitSet);
      REQUIRE(bitSet == 1);
      bitutil::toggle(&dummyByte, pos);
      bitutil::get(&dummyByte, pos, bitSet);
      REQUIRE(bitSet == 0);
    } else {
      bitutil::get(&dummyByte, pos, bitSet);
      REQUIRE(bitSet == false);
      bitutil::toggle(&dummyByte, pos);
      bitutil::get(&dummyByte, pos, bitSet);
      REQUIRE(bitSet == true);
    }
  }
  REQUIRE(dummyByte == 0x0f);

  dummyByte = 0;
  bitutil::set(&dummyByte, 8);
  REQUIRE(dummyByte == 0);
  bitutil::set(&dummyByte, -1);
  REQUIRE(dummyByte == 0);
  dummyByte = 0xff;
  bitutil::clear(&dummyByte, 8);
  REQUIRE(dummyByte == 0xff);
  bitutil::clear(&dummyByte, -1);
  REQUIRE(dummyByte == 0xff);
  dummyByte = 0x00;
  bitutil::toggle(&dummyByte, 8);
  REQUIRE(dummyByte == 0x00);
  bitutil::toggle(&dummyByte, -1);
  REQUIRE(dummyByte == 0x00);

  REQUIRE(bitutil::get(&dummyByte, 8, bitSet) == false);
}

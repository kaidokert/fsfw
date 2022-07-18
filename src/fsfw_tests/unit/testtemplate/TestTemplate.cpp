#include <catch2/catch_test_macros.hpp>

#include "fsfw_tests/unit/CatchDefinitions.h"

/**
 * @brief 	Template test file
 * @details
 * In each test case, the code outside the sections is executed
 * for EACH section.
 * The most common macros to perform tests are:
 *  - CHECK(...): assert expression and continues even if it fails
 *  - REQUIRE(...): test case fails if assertion fails
 *
 *  Tests are generally sturctured in test cases and sections, see example
 *  below.
 *
 *  More Documentation:
 *  - https://github.com/catchorg/Catch2
 *  - https://github.com/catchorg/Catch2/blob/master/docs/assertions.md
 *  - https://github.com/catchorg/Catch2/blob/master/docs/test-cases-and-sections.md
 */
TEST_CASE("Dummy Test", "[DummyTest]") {
  uint8_t testVariable = 1;
  // perform set-up here
  CHECK(testVariable == 1);
  SECTION("TestSection") {
    // set-up is run for each section
    REQUIRE(testVariable == 1);
  }
  // perform tear-down here
}

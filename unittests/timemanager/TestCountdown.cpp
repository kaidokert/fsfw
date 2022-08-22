#include <fsfw/timemanager/Countdown.h>

#include <catch2/catch_test_macros.hpp>

#include "CatchDefinitions.h"

TEST_CASE("Countdown Tests", "[TestCountdown]") {
  INFO("Countdown Tests");
  Countdown count(20);
  REQUIRE(count.timeout == 20);
  REQUIRE(count.setTimeout(100) == static_cast<uint16_t>(returnvalue::OK));
  REQUIRE(count.timeout == 100);
  REQUIRE(count.setTimeout(150) == static_cast<uint16_t>(returnvalue::OK));
  REQUIRE(count.isBusy());
  REQUIRE(not count.hasTimedOut());
  uint32_t number = count.getRemainingMillis();
  REQUIRE(number > 0);
  bool blocked = false;
  while (not count.hasTimedOut()) {
    blocked = true;
  };
  REQUIRE(blocked);
  number = count.getRemainingMillis();
  REQUIRE(number == 0);
  count.resetTimer();
  REQUIRE(not count.hasTimedOut());
  REQUIRE(count.isBusy());
}

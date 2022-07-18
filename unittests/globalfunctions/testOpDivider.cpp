#include <catch2/catch_test_macros.hpp>

#include "fsfw/globalfunctions/PeriodicOperationDivider.h"

TEST_CASE("OpDivider", "[OpDivider]") {
  auto opDivider = PeriodicOperationDivider(1);
  REQUIRE(opDivider.getDivider() == 1);
  REQUIRE(opDivider.getCounter() == 1);
  REQUIRE(opDivider.check() == true);
  REQUIRE(opDivider.checkAndIncrement() == true);
  REQUIRE(opDivider.getCounter() == 1);
  REQUIRE(opDivider.check() == true);
  REQUIRE(opDivider.checkAndIncrement() == true);
  REQUIRE(opDivider.checkAndIncrement() == true);

  opDivider.setDivider(0);
  REQUIRE(opDivider.getCounter() == 1);
  REQUIRE(opDivider.checkAndIncrement() == true);
  REQUIRE(opDivider.getCounter() == 1);
  REQUIRE(opDivider.checkAndIncrement() == true);
  REQUIRE(opDivider.checkAndIncrement() == true);

  opDivider.setDivider(2);
  opDivider.resetCounter();
  REQUIRE(opDivider.getDivider() == 2);
  REQUIRE(opDivider.getCounter() == 1);
  REQUIRE(opDivider.check() == false);
  REQUIRE(opDivider.checkAndIncrement() == false);
  REQUIRE(opDivider.getCounter() == 2);
  REQUIRE(opDivider.check() == true);
  REQUIRE(opDivider.checkAndIncrement() == true);
  REQUIRE(opDivider.getCounter() == 1);
  REQUIRE(opDivider.check() == false);
  REQUIRE(opDivider.checkAndIncrement() == false);
  REQUIRE(opDivider.getCounter() == 2);
  REQUIRE(opDivider.checkAndIncrement() == true);
  REQUIRE(opDivider.checkAndIncrement() == false);
  REQUIRE(opDivider.checkAndIncrement() == true);
  REQUIRE(opDivider.checkAndIncrement() == false);

  opDivider.setDivider(3);
  opDivider.resetCounter();
  REQUIRE(opDivider.checkAndIncrement() == false);
  REQUIRE(opDivider.checkAndIncrement() == false);
  REQUIRE(opDivider.getCounter() == 3);
  REQUIRE(opDivider.checkAndIncrement() == true);
  REQUIRE(opDivider.getCounter() == 1);
  REQUIRE(opDivider.checkAndIncrement() == false);

  auto opDividerNonResetting = PeriodicOperationDivider(2, false);
  REQUIRE(opDividerNonResetting.getCounter() == 1);
  REQUIRE(opDividerNonResetting.check() == false);
  REQUIRE(opDividerNonResetting.checkAndIncrement() == false);
  REQUIRE(opDividerNonResetting.getCounter() == 2);
  REQUIRE(opDividerNonResetting.check() == true);
  REQUIRE(opDividerNonResetting.checkAndIncrement() == true);
  REQUIRE(opDividerNonResetting.getCounter() == 3);
  REQUIRE(opDividerNonResetting.checkAndIncrement() == true);
  REQUIRE(opDividerNonResetting.getCounter() == 4);
  opDividerNonResetting.resetCounter();
  REQUIRE(opDividerNonResetting.getCounter() == 1);
  REQUIRE(opDividerNonResetting.check() == false);
  REQUIRE(opDividerNonResetting.checkAndIncrement() == false);
  REQUIRE(opDividerNonResetting.getCounter() == 2);
}

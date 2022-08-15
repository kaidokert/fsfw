#include <fsfw/power/DummyPowerSwitcher.h>
#include <fsfw/power/PowerSwitcher.h>

#include <catch2/catch_test_macros.hpp>

#include "mocks/PowerSwitcherMock.h"
#include "objects/systemObjectList.h"

TEST_CASE("Power Switcher", "[power-switcher]") {
  PowerSwitcherMock mock;
  PowerSwitcher switcher(&mock, 1);
  DummyPowerSwitcher dummySwitcher(objects::DUMMY_POWER_SWITCHER, 5, 5, false);
  PowerSwitcher switcherUsingDummy(&dummySwitcher, 1);
  SwitchInfo switchInfo;
  mock.initSwitch(1);

  SECTION("Basic Tests") {
    REQUIRE(switcher.getFirstSwitch() == 1);
    REQUIRE(switcher.getSecondSwitch() == power::NO_SWITCH);
    // Default start state
    REQUIRE(switcher.getState() == PowerSwitcher::SWITCH_IS_OFF);
    switcher.turnOn(true);
    REQUIRE(mock.getAmountSwitchStatWasRequested() == 1);
    REQUIRE(switcher.getState() == PowerSwitcher::WAIT_ON);
    REQUIRE(switcher.checkSwitchState() == PowerSwitcher::IN_POWER_TRANSITION);
    REQUIRE(switcher.active());
    switcher.doStateMachine();
    REQUIRE(switcher.getState() == PowerSwitcher::SWITCH_IS_ON);
    mock.getSwitchInfo(1, switchInfo);
    REQUIRE(switchInfo.timesCalledOn == 1);
    REQUIRE(not switcher.active());
    REQUIRE(mock.getAmountSwitchStatWasRequested() == 2);
    REQUIRE(switcher.checkSwitchState() == returnvalue::OK);
    REQUIRE(mock.getAmountSwitchStatWasRequested() == 3);
    switcher.turnOff(false);
    REQUIRE(mock.getAmountSwitchStatWasRequested() == 3);
    REQUIRE(switcher.getState() == PowerSwitcher::WAIT_OFF);
    REQUIRE(switcher.active());
    REQUIRE(switcher.getState() == PowerSwitcher::WAIT_OFF);
    switcher.doStateMachine();
    mock.getSwitchInfo(1, switchInfo);
    REQUIRE(switcher.getState() == PowerSwitcher::SWITCH_IS_OFF);
    REQUIRE(switchInfo.timesCalledOn == 1);
    REQUIRE(switchInfo.timesCalledOff == 1);
    REQUIRE(not switcher.active());
    REQUIRE(mock.getAmountSwitchStatWasRequested() == 4);
  }

  SECTION("Dummy Test") {
    // Same tests, but we can't really check the dummy
    REQUIRE(switcherUsingDummy.getFirstSwitch() == 1);
    REQUIRE(switcherUsingDummy.getSecondSwitch() == power::NO_SWITCH);
    REQUIRE(switcherUsingDummy.getState() == PowerSwitcher::SWITCH_IS_OFF);
    switcherUsingDummy.turnOn(true);
    REQUIRE(switcherUsingDummy.getState() == PowerSwitcher::WAIT_ON);
    REQUIRE(switcherUsingDummy.active());
    switcherUsingDummy.doStateMachine();
    REQUIRE(switcherUsingDummy.getState() == PowerSwitcher::SWITCH_IS_ON);
    REQUIRE(not switcherUsingDummy.active());

    switcherUsingDummy.turnOff(false);
    REQUIRE(switcherUsingDummy.getState() == PowerSwitcher::WAIT_OFF);
    REQUIRE(switcherUsingDummy.active());
    REQUIRE(switcherUsingDummy.getState() == PowerSwitcher::WAIT_OFF);
    switcherUsingDummy.doStateMachine();
    REQUIRE(switcherUsingDummy.getState() == PowerSwitcher::SWITCH_IS_OFF);
    REQUIRE(not switcherUsingDummy.active());
  }

  SECTION("More Dummy Tests") {}
}

#include "PowerSwitcherMock.h"

static uint32_t SWITCH_REQUEST_UPDATE_VALUE = 0;

PowerSwitcherMock::PowerSwitcherMock() {}

ReturnValue_t PowerSwitcherMock::sendSwitchCommand(power::Switch_t switchNr, ReturnValue_t onOff) {
  if (switchMap.count(switchNr) == 0) {
    switchMap.emplace(switchNr, SwitchInfo(switchNr, onOff));
  } else {
    SwitchInfo& info = switchMap.at(switchNr);
    info.currentState = onOff;
    if (onOff == PowerSwitchIF::SWITCH_ON) {
      info.timesCalledOn++;
    } else {
      info.timesCalledOff++;
    }
  }
  return returnvalue::OK;
}

ReturnValue_t PowerSwitcherMock::sendFuseOnCommand(uint8_t fuseNr) {
  if (fuseMap.count(fuseNr) == 0) {
    fuseMap.emplace(fuseNr, FuseInfo(fuseNr));
  } else {
    FuseInfo& info = fuseMap.at(fuseNr);
    info.timesCalled++;
  }
  return returnvalue::OK;
}

ReturnValue_t PowerSwitcherMock::getSwitchState(power::Switch_t switchNr) const {
  if (switchMap.count(switchNr) == 1) {
    auto& info = switchMap.at(switchNr);
    SWITCH_REQUEST_UPDATE_VALUE++;
    return info.currentState;
  }
  return returnvalue::FAILED;
}

ReturnValue_t PowerSwitcherMock::getFuseState(uint8_t fuseNr) const {
  if (fuseMap.count(fuseNr) == 1) {
    return FUSE_ON;
  } else {
    return FUSE_OFF;
  }
  return returnvalue::FAILED;
}

uint32_t PowerSwitcherMock::getSwitchDelayMs(void) const { return 5000; }

SwitchInfo::SwitchInfo() : switcher(0) {}

SwitchInfo::SwitchInfo(power::Switch_t switcher, ReturnValue_t initState)
    : switcher(switcher), currentState(initState) {}

FuseInfo::FuseInfo(uint8_t fuse) : fuse(fuse) {}

void PowerSwitcherMock::getSwitchInfo(power::Switch_t switcher, SwitchInfo& info) {
  if (switchMap.count(switcher) == 1) {
    info = switchMap.at(switcher);
  }
}

void PowerSwitcherMock::getFuseInfo(uint8_t fuse, FuseInfo& info) {
  if (fuseMap.count(fuse) == 1) {
    info = fuseMap.at(fuse);
  }
}

uint32_t PowerSwitcherMock::getAmountSwitchStatWasRequested() {
  return SWITCH_REQUEST_UPDATE_VALUE;
}

void PowerSwitcherMock::initSwitch(power::Switch_t switchNr) {
  switchMap.emplace(switchNr, SwitchInfo(switchNr, PowerSwitchIF::SWITCH_OFF));
}

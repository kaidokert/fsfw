#include "DummyPowerSwitcher.h"

DummyPowerSwitcher::DummyPowerSwitcher(object_id_t objectId, size_t numberOfSwitches,
                                       size_t numberOfFuses, uint32_t switchDelayMs)
    : SystemObject(objectId),
      switcherList(numberOfSwitches),
      fuseList(numberOfFuses),
      switchDelayMs(switchDelayMs) {}

void DummyPowerSwitcher::setInitialSwitcherList(std::vector<ReturnValue_t> switcherList) {
  this->switcherList = switcherList;
}

void DummyPowerSwitcher::setInitialFusesList(std::vector<ReturnValue_t> fuseList) {
  this->fuseList = fuseList;
}

ReturnValue_t DummyPowerSwitcher::sendSwitchCommand(power::Switch_t switchNr, ReturnValue_t onOff) {
  if (switchNr < switcherList.capacity()) {
    switcherList[switchNr] = onOff;
  }
  return RETURN_FAILED;
}

ReturnValue_t DummyPowerSwitcher::sendFuseOnCommand(uint8_t fuseNr) {
  if (fuseNr < fuseList.capacity()) {
    fuseList[fuseNr] = FUSE_ON;
  }
  return RETURN_FAILED;
}

ReturnValue_t DummyPowerSwitcher::getSwitchState(power::Switch_t switchNr) const {
  if (switchNr < switcherList.capacity()) {
    return switcherList[switchNr];
  }
  return HasReturnvaluesIF::RETURN_FAILED;
}

ReturnValue_t DummyPowerSwitcher::getFuseState(uint8_t fuseNr) const {
  if (fuseNr < fuseList.capacity()) {
    return fuseList[fuseNr];
  }
  return HasReturnvaluesIF::RETURN_FAILED;
}

uint32_t DummyPowerSwitcher::getSwitchDelayMs(void) const { return switchDelayMs; }

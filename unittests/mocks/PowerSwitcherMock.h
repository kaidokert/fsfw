#ifndef FSFW_TESTS_SRC_FSFW_TESTS_UNIT_MOCKS_POWERSWITCHERMOCK_H_
#define FSFW_TESTS_SRC_FSFW_TESTS_UNIT_MOCKS_POWERSWITCHERMOCK_H_

#include <fsfw/power/PowerSwitchIF.h>

#include <map>
#include <utility>

struct SwitchInfo {
 public:
  SwitchInfo();
  SwitchInfo(power::Switch_t switcher, ReturnValue_t initState);

  power::Switch_t switcher;
  ReturnValue_t currentState = PowerSwitchIF::SWITCH_OFF;
  uint32_t timesCalledOn = 0;
  uint32_t timesCalledOff = 0;
  uint32_t timesStatusRequested = 0;
};

struct FuseInfo {
 public:
  FuseInfo(uint8_t fuse);
  uint8_t fuse;
  uint32_t timesCalled = 0;
};

class PowerSwitcherMock : public PowerSwitchIF {
 public:
  PowerSwitcherMock();

  ReturnValue_t sendSwitchCommand(power::Switch_t switchNr, ReturnValue_t onOff) override;
  ReturnValue_t sendFuseOnCommand(uint8_t fuseNr) override;
  ReturnValue_t getSwitchState(power::Switch_t switchNr) const override;
  ReturnValue_t getFuseState(uint8_t fuseNr) const override;
  uint32_t getSwitchDelayMs(void) const override;

  void getSwitchInfo(power::Switch_t switcher, SwitchInfo& info);
  void getFuseInfo(uint8_t fuse, FuseInfo& info);

  uint32_t getAmountSwitchStatWasRequested();

  void initSwitch(power::Switch_t switchNr);

 private:
  using SwitchOnOffPair = std::pair<power::Switch_t, ReturnValue_t>;
  using FuseOnOffPair = std::pair<uint8_t, ReturnValue_t>;
  std::map<power::Switch_t, SwitchInfo> switchMap;
  std::map<uint8_t, FuseInfo> fuseMap;
};

#endif /* FSFW_TESTS_SRC_FSFW_TESTS_UNIT_MOCKS_POWERSWITCHERMOCK_H_ */

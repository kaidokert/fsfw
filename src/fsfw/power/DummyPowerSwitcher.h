#ifndef FSFW_SRC_FSFW_POWER_DUMMYPOWERSWITCHER_H_
#define FSFW_SRC_FSFW_POWER_DUMMYPOWERSWITCHER_H_

#include <cstddef>
#include <vector>

#include "PowerSwitchIF.h"
#include "definitions.h"
#include "fsfw/objectmanager/SystemObject.h"

/**
 * @brief   This component can be used to simulate a power switcher like a
 *          Power Control Distribution Unit (PCDU)
 * @details
 * The dummy switcher will simply cache the commanded fuse and switch states and return them
 * in the according switch getter functions. In that sense, it simulates an ideal PCDU.
 */
class DummyPowerSwitcher : public SystemObject, public PowerSwitchIF {
 public:
  DummyPowerSwitcher(object_id_t objectId, size_t numberOfSwitches, size_t numberOfFuses,
                     bool registerGlobally = true, uint32_t switchDelayMs = 5000);

  void setInitialSwitcherList(std::vector<ReturnValue_t> switcherList);
  void setInitialFusesList(std::vector<ReturnValue_t> switcherList);

  virtual ReturnValue_t sendSwitchCommand(power::Switch_t switchNr, ReturnValue_t onOff) override;
  virtual ReturnValue_t sendFuseOnCommand(uint8_t fuseNr) override;
  virtual ReturnValue_t getSwitchState(power::Switch_t switchNr) const override;
  virtual ReturnValue_t getFuseState(uint8_t fuseNr) const override;
  virtual uint32_t getSwitchDelayMs(void) const override;

 private:
  std::vector<ReturnValue_t> switcherList;
  std::vector<ReturnValue_t> fuseList;
  uint32_t switchDelayMs = 5000;
};

#endif /* FSFW_SRC_FSFW_POWER_DUMMYPOWERSWITCHER_H_ */

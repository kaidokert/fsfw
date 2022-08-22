#ifndef _FSFW_POWER_POWERSWITCHERCOMPONENT_H_
#define _FSFW_POWER_POWERSWITCHERCOMPONENT_H_

#include <fsfw/health/HasHealthIF.h>
#include <fsfw/health/HealthHelper.h>
#include <fsfw/modes/HasModesIF.h>
#include <fsfw/modes/ModeHelper.h>
#include <fsfw/objectmanager/SystemObject.h>
#include <fsfw/power/PowerSwitcher.h>
#include <fsfw/power/definitions.h>
#include <fsfw/tasks/ExecutableObjectIF.h>

class PowerSwitchIF;

/**
 * @brief Allows to create an power switch object with its own mode and health
 * @details
 * This basic component allows to create an object which is solely responsible for managing a
 * switch. It also has a mode and a health by implementing the respective interface components
 * which allows integrating this component into a system mode tree.
 *
 * Commanding this component to MODE_OFF will cause the switcher to turn the switch off while
 * commanding in to MODE_ON will cause the switcher to turn the switch on.
 */
class PowerSwitcherComponent : public SystemObject,
                               public ExecutableObjectIF,
                               public HasModesIF,
                               public HasHealthIF {
 public:
  PowerSwitcherComponent(object_id_t objectId, PowerSwitchIF *pwrSwitcher,
                         power::Switch_t pwrSwitch);

 private:
  MessageQueueIF *queue = nullptr;
  PowerSwitcher switcher;

  Mode_t mode = MODE_OFF;
  Submode_t submode = 0;

  ModeHelper modeHelper;
  HealthHelper healthHelper;

  void setMode(Mode_t newMode, Submode_t newSubmode);

  virtual ReturnValue_t performOperation(uint8_t opCode) override;

  ReturnValue_t initialize() override;

  MessageQueueId_t getCommandQueue() const override;
  void getMode(Mode_t *mode, Submode_t *submode) override;
  ReturnValue_t checkModeCommand(Mode_t mode, Submode_t submode,
                                 uint32_t *msToReachTheMode) override;
  void startTransition(Mode_t mode, Submode_t submode) override;
  void setToExternalControl() override;
  void announceMode(bool recursive) override;

  ReturnValue_t setHealth(HealthState health) override;
  HasHealthIF::HealthState getHealth() override;
};

#endif /* _FSFW_POWER_POWERSWITCHERCOMPONENT_H_ */

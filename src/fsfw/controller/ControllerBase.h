#ifndef FSFW_CONTROLLER_CONTROLLERBASE_H_
#define FSFW_CONTROLLER_CONTROLLERBASE_H_

#include "fsfw/health/HasHealthIF.h"
#include "fsfw/health/HealthHelper.h"
#include "fsfw/modes/HasModesIF.h"
#include "fsfw/modes/ModeHelper.h"
#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/tasks/ExecutableObjectIF.h"
#include "fsfw/tasks/PeriodicTaskIF.h"

/**
 * @brief   Generic base class for controller classes
 * @details
 * Implements common interfaces for controllers, which generally have
 * a mode and a health state. This avoids boilerplate code.
 */
class ControllerBase : public HasModesIF,
                       public HasHealthIF,
                       public ExecutableObjectIF,
                       public SystemObject {
 public:
  static const Mode_t MODE_NORMAL = 2;

  ControllerBase(object_id_t setObjectId, object_id_t parentId, size_t commandQueueDepth = 3);
  ~ControllerBase() override;

  /** SystemObject override */
  ReturnValue_t initialize() override;

  [[nodiscard]] MessageQueueId_t getCommandQueue() const override;

  /** HasHealthIF overrides */
  ReturnValue_t setHealth(HealthState health) override;
  HasHealthIF::HealthState getHealth() override;

  /** ExecutableObjectIF overrides */
  ReturnValue_t performOperation(uint8_t opCode) override;
  void setTaskIF(PeriodicTaskIF *task) override;
  ReturnValue_t initializeAfterTaskCreation() override;

 protected:
  /**
   * Implemented by child class. Handle command messages which are not
   * mode or health messages.
   * @param message
   * @return
   */
  virtual ReturnValue_t handleCommandMessage(CommandMessage *message) = 0;

  /**
   * Periodic helper, implemented by child class.
   */
  virtual void performControlOperation() = 0;

  ReturnValue_t checkModeCommand(Mode_t mode, Submode_t submode,
                                 uint32_t *msToReachTheMode) override = 0;

  const object_id_t parentId;

  Mode_t mode;

  Submode_t submode;

  MessageQueueIF *commandQueue = nullptr;

  ModeHelper modeHelper;

  HealthHelper healthHelper;

  /**
   * Pointer to the task which executes this component,
   * is invalid before setTaskIF was called.
   */
  PeriodicTaskIF *executingTask = nullptr;

  /** Handle mode and health messages */
  virtual void handleQueue();

  /** Mode helpers */
  virtual void modeChanged(Mode_t mode, Submode_t submode);
  void startTransition(Mode_t mode, Submode_t submode) override;
  void getMode(Mode_t *mode, Submode_t *submode) override;
  void setToExternalControl() override;
  void announceMode(bool recursive) override;
  /** HK helpers */
  virtual void changeHK(Mode_t mode, Submode_t submode, bool enable);
};

#endif /* FSFW_CONTROLLER_CONTROLLERBASE_H_ */

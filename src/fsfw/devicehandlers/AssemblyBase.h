#ifndef FSFW_DEVICEHANDLERS_ASSEMBLYBASE_H_
#define FSFW_DEVICEHANDLERS_ASSEMBLYBASE_H_

#include "../container/FixedArrayList.h"
#include "../subsystem/SubsystemBase.h"
#include "DeviceHandlerBase.h"

/**
 * @brief   Base class to implement reconfiguration and failure handling for
 *          redundant devices by monitoring their modes and health states.
 * @details
 * Documentation: Dissertation Baetz p.156, 157.
 *
 * This class reduces the complexity of controller components which would
 * otherwise be needed for the handling of redundant devices.
 *
 * The template class monitors mode and health state of its children
 * and checks availability of devices on every detected change.
 * AssemblyBase does not implement any redundancy logic by itself, but provides
 * adaptation points for implementations to do so. Since most monitoring
 * activities rely on mode and health state only and are therefore
 * generic, it is sufficient for subclasses to provide:
 *
 * 1. check logic when active-> checkChildrenStateOn
 * 2. transition logic to change the mode -> commandChildren
 *
 * Important:
 *
 * The implementation must call registerChild(object_id_t child)
 * for all commanded children during initialization.
 * The implementation must call the initialization function of the base class.
 * (This will call the function in SubsystemBase)
 *
 */
class AssemblyBase : public SubsystemBase {
 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::ASSEMBLY_BASE;
  static const ReturnValue_t NEED_SECOND_STEP = MAKE_RETURN_CODE(0x01);
  static const ReturnValue_t NEED_TO_RECONFIGURE = MAKE_RETURN_CODE(0x02);
  static const ReturnValue_t MODE_FALLBACK = MAKE_RETURN_CODE(0x03);
  static const ReturnValue_t CHILD_NOT_COMMANDABLE = MAKE_RETURN_CODE(0x04);
  static const ReturnValue_t NEED_TO_CHANGE_HEALTH = MAKE_RETURN_CODE(0x05);
  static const ReturnValue_t NOT_ENOUGH_CHILDREN_IN_CORRECT_STATE = MAKE_RETURN_CODE(0xa1);

  AssemblyBase(object_id_t objectId, object_id_t parentId, uint16_t commandQueueDepth = 8);
  virtual ~AssemblyBase();

 protected:
  /**
   * Command children to reach [mode,submode] combination
   * Can be done by setting #commandsOutstanding correctly,
   * or using executeTable()
   * @param mode
   * @param submode
   * @return
   *    - @c returnvalue::OK if ok
   *    - @c NEED_SECOND_STEP if children need to be commanded again
   */
  virtual ReturnValue_t commandChildren(Mode_t mode, Submode_t submode) = 0;

  /**
   * Check whether desired assembly mode was achieved by checking the modes
   * or/and health states of child device handlers.
   * The assembly template class will also call this function if a health
   * or mode change of a child device handler was detected.
   * @param wantedMode
   * @param wantedSubmode
   * @return
   */
  virtual ReturnValue_t checkChildrenStateOn(Mode_t wantedMode, Submode_t wantedSubmode) = 0;

  /**
   * Check whether a combination of mode and submode is valid.
   *
   * Ground Controller like precise return values from HasModesIF.
   * So, please return any of them.
   *
   * @param mode The targeted mode
   * @param submode The targeted submmode
   * @return Any information why this combination is invalid from HasModesIF
   * 			like HasModesIF::INVALID_SUBMODE.
   * 			On success return returnvalue::OK
   */
  virtual ReturnValue_t isModeCombinationValid(Mode_t mode, Submode_t submode) = 0;

  enum InternalState {
    STATE_NONE,
    STATE_OVERWRITE_HEALTH,
    STATE_NEED_SECOND_STEP,
    STATE_SINGLE_STEP,
    STATE_SECOND_STEP,
  } internalState;

  enum RecoveryState {
    RECOVERY_IDLE,
    RECOVERY_STARTED,
    RECOVERY_ONGOING,
    RECOVERY_ONGOING_2,
    RECOVERY_WAIT
  } recoveryState;  //!< Indicates if one of the children requested a recovery.
  ChildrenMap::iterator recoveringDevice;

  /**
   * the mode the current transition is trying to achieve.
   * Can be different from the modehelper.commandedMode!
   */
  Mode_t targetMode;

  /**
   * the submode the current transition is trying to achieve.
   * Can be different from the modehelper.commandedSubmode!
   */
  Submode_t targetSubmode;

  Countdown recoveryOffTimer;

  static const uint32_t POWER_OFF_TIME_MS = 1000;

  virtual ReturnValue_t handleCommandMessage(CommandMessage *message);

  virtual ReturnValue_t handleHealthReply(CommandMessage *message);

  virtual void performChildOperation();

  bool handleChildrenChanged();

  /**
   * This method is called if the children changed its mode in a way that
   * the current mode can't be kept.
   * Default behavior is to go to MODE_OFF.
   * @param result The failure code which was returned by checkChildrenState.
   */
  virtual void handleChildrenLostMode(ReturnValue_t result);

  bool handleChildrenChangedHealth();

  virtual void handleChildrenTransition();

  ReturnValue_t checkModeCommand(Mode_t mode, Submode_t submode, uint32_t *msToReachTheMode);

  virtual void startTransition(Mode_t mode, Submode_t submode);

  virtual void doStartTransition(Mode_t mode, Submode_t submode);

  virtual bool isInTransition();

  virtual void handleModeReached();

  virtual void handleModeTransitionFailed(ReturnValue_t result);

  void sendHealthCommand(MessageQueueId_t sendTo, HealthState health);

  virtual ReturnValue_t checkChildrenStateOff();

  ReturnValue_t checkChildrenState();

  virtual ReturnValue_t checkChildOff(uint32_t objectId);

  /**
   * Manages recovery of a device
   * @return true if recovery is still ongoing, false else.
   */
  bool checkAndHandleRecovery();

  /**
   * Helper method to overwrite health state of one of the children.
   * Also sets state to STATE_OVERWRITE_HEATH.
   * @param objectId Must be a registered child.
   */
  void overwriteDeviceHealth(object_id_t objectId, HasHealthIF::HealthState oldHealth);
};

#endif /* FSFW_DEVICEHANDLERS_ASSEMBLYBASE_H_ */

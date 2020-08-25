#ifndef ASSEMBLYBASE_H_
#define ASSEMBLYBASE_H_

#include "../container/FixedArrayList.h"
#include "DeviceHandlerBase.h"
#include "../subsystem/SubsystemBase.h"

class AssemblyBase: public SubsystemBase {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::ASSEMBLY_BASE;
	static const ReturnValue_t NEED_SECOND_STEP = MAKE_RETURN_CODE(0x01);
	static const ReturnValue_t NEED_TO_RECONFIGURE = MAKE_RETURN_CODE(0x02);
	static const ReturnValue_t MODE_FALLBACK = MAKE_RETURN_CODE(0x03);
	static const ReturnValue_t CHILD_NOT_COMMANDABLE = MAKE_RETURN_CODE(0x04);
	static const ReturnValue_t NEED_TO_CHANGE_HEALTH = MAKE_RETURN_CODE(0x05);
	static const ReturnValue_t NOT_ENOUGH_CHILDREN_IN_CORRECT_STATE =
			MAKE_RETURN_CODE(0xa1);

	AssemblyBase(object_id_t objectId, object_id_t parentId, uint16_t commandQueueDepth = 8);
	virtual ~AssemblyBase();

protected:
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
	} recoveryState; //!< Indicates if one of the children requested a recovery.
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
	 * This method is called if the children changed its mode in a way that the current
	 * mode can't be kept.
	 * Default behavior is to go to MODE_OFF.
	 * @param result The failure code which was returned by checkChildrenState.
	 */
	virtual void handleChildrenLostMode(ReturnValue_t result);

	bool handleChildrenChangedHealth();

	virtual void handleChildrenTransition();

	ReturnValue_t checkModeCommand(Mode_t mode, Submode_t submode,
			uint32_t *msToReachTheMode);

	virtual ReturnValue_t isModeCombinationValid(Mode_t mode,
			Submode_t submode) = 0;

	virtual void startTransition(Mode_t mode, Submode_t submode);

	virtual void doStartTransition(Mode_t mode, Submode_t submode);

	virtual bool isInTransition();

	virtual void handleModeReached();

	virtual void handleModeTransitionFailed(ReturnValue_t result);

	void sendHealthCommand(MessageQueueId_t sendTo, HealthState health);

	//SHOULDDO: Change that OVERWRITE_HEALTH may be returned (or return internalState directly?)
	/**
	 * command children to reach mode,submode
	 *
	 * set #commandsOutstanding correctly, or use executeTable()
	 *
	 * @param mode
	 * @param submode
	 * @return
	 *    - @c RETURN_OK if ok
	 *    - @c NEED_SECOND_STEP if children need to be commanded again
	 */
	virtual ReturnValue_t commandChildren(Mode_t mode, Submode_t submode) = 0;

	//SHOULDDO: Remove wantedMode, wantedSubmode, as targetMode/submode is available?
	virtual ReturnValue_t checkChildrenStateOn(Mode_t wantedMode,
			Submode_t wantedSubmode) = 0;

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

#endif /* ASSEMBLYBASE_H_ */

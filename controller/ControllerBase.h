#ifndef FSFW_CONTROLLER_CONTROLLERBASE_H_
#define FSFW_CONTROLLER_CONTROLLERBASE_H_

#include "../health/HasHealthIF.h"
#include "../health/HealthHelper.h"
#include "../modes/HasModesIF.h"
#include "../modes/ModeHelper.h"
#include "../objectmanager/SystemObject.h"
#include "../tasks/ExecutableObjectIF.h"
#include "../datapool/HkSwitchHelper.h"

/**
 * @brief  Generic base class for controller classes
 * @details
 * Implements common interfaces for controllers.
 */
class ControllerBase: public HasModesIF,
		public HasHealthIF,
		public ExecutableObjectIF,
		public SystemObject,
		public HasReturnvaluesIF {
public:
	static const Mode_t MODE_NORMAL = 2;

	ControllerBase(object_id_t setObjectId, object_id_t parentId,
			size_t commandQueueDepth = 3);
	virtual ~ControllerBase();

	virtual ReturnValue_t initialize() override;

	virtual MessageQueueId_t getCommandQueue() const override;

	virtual ReturnValue_t performOperation(uint8_t opCode) override;

	virtual ReturnValue_t setHealth(HealthState health) override;

	virtual HasHealthIF::HealthState getHealth() override;

	/**
	 * Implementation of ExecutableObjectIF function
	 *
	 * Used to setup the reference of the task, that executes this component
	 * @param task_ Pointer to the taskIF of this task
	 */
	virtual  void setTaskIF(PeriodicTaskIF* task_) override;

	virtual ReturnValue_t initializeAfterTaskCreation() override;

protected:

	/**
	 * Implemented by child class. Handle command messages which are not
	 * mode or health messages.
	 * @param message
	 * @return
	 */
    virtual ReturnValue_t handleCommandMessage(CommandMessage *message) = 0;

    virtual void performControlOperation() = 0;

    virtual ReturnValue_t checkModeCommand(Mode_t mode, Submode_t submode,
            uint32_t *msToReachTheMode) = 0;

	const object_id_t parentId;

	Mode_t mode;

	Submode_t submode;

	MessageQueueIF* commandQueue = nullptr;

	ModeHelper modeHelper;

	HealthHelper healthHelper;

	HkSwitchHelper hkSwitcher;

	/**
	 * Pointer to the task which executes this component,
	 * is invalid before setTaskIF was called.
	 */
	PeriodicTaskIF* executingTask = nullptr;

	void handleQueue();

	virtual void modeChanged(Mode_t mode, Submode_t submode);
	virtual void startTransition(Mode_t mode, Submode_t submode);
	virtual void getMode(Mode_t *mode, Submode_t *submode);
	virtual void setToExternalControl();
	virtual void announceMode(bool recursive);
	virtual void changeHK(Mode_t mode, Submode_t submode, bool enable);
};

#endif /* FSFW_CONTROLLER_CONTROLLERBASE_H_ */

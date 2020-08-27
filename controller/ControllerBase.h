#ifndef CONTROLLERBASE_H_
#define CONTROLLERBASE_H_

#include "../health/HasHealthIF.h"
#include "../health/HealthHelper.h"
#include "../modes/HasModesIF.h"
#include "../modes/ModeHelper.h"
#include "../objectmanager/SystemObject.h"
#include "../tasks/ExecutableObjectIF.h"
#include "../datapool/HkSwitchHelper.h"


class ControllerBase: public HasModesIF,
		public HasHealthIF,
		public ExecutableObjectIF,
		public SystemObject,
		public HasReturnvaluesIF {
public:

	static const Mode_t MODE_NORMAL = 2;

	ControllerBase(uint32_t setObjectId, uint32_t parentId,
			size_t commandQueueDepth = 3);
	virtual ~ControllerBase();

	ReturnValue_t initialize();

	virtual MessageQueueId_t getCommandQueue() const;

	virtual ReturnValue_t performOperation(uint8_t opCode);

	virtual ReturnValue_t setHealth(HealthState health);

	virtual HasHealthIF::HealthState getHealth();

	/**
	 * Implementation of ExecutableObjectIF function
	 *
	 * Used to setup the reference of the task, that executes this component
	 * @param task_ Pointer to the taskIF of this task
	 */
	virtual  void setTaskIF(PeriodicTaskIF* task_);


protected:
	const uint32_t parentId;

	Mode_t mode;

	Submode_t submode;

	MessageQueueIF* commandQueue;

	ModeHelper modeHelper;

	HealthHelper healthHelper;

	HkSwitchHelper hkSwitcher;

	/**
	 * Pointer to the task which executes this component, is invalid before setTaskIF was called.
	 */
	PeriodicTaskIF* executingTask;

	void handleQueue();

	virtual ReturnValue_t handleCommandMessage(CommandMessage *message) = 0;
	virtual void performControlOperation() = 0;
	virtual ReturnValue_t checkModeCommand(Mode_t mode, Submode_t submode,
			uint32_t *msToReachTheMode) = 0;
	virtual void modeChanged(Mode_t mode, Submode_t submode);
	virtual void startTransition(Mode_t mode, Submode_t submode);
	virtual void getMode(Mode_t *mode, Submode_t *submode);
	virtual void setToExternalControl();
	virtual void announceMode(bool recursive);
	virtual void changeHK(Mode_t mode, Submode_t submode, bool enable);
};

#endif /* CONTROLLERBASE_H_ */

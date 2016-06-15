#ifndef CONTROLLERBASE_H_
#define CONTROLLERBASE_H_

#include <framework/health/HasHealthIF.h>
#include <framework/health/HealthHelper.h>
#include <framework/modes/HasModesIF.h>
#include <framework/modes/ModeHelper.h>
#include <framework/objectmanager/SystemObject.h>
#include <framework/tasks/ExecutableObjectIF.h>

class ControllerBase: public HasModesIF,
		public HasHealthIF,
		public ExecutableObjectIF,
		public SystemObject,
		public HasReturnvaluesIF {
public:

	static const Mode_t MODE_NORMAL = 2;

	ControllerBase(uint32_t setObjectId, uint32_t parentId, size_t commandQueueDepth = 3);
	virtual ~ControllerBase();

	ReturnValue_t initialize();

	virtual MessageQueueId_t getCommandQueue() const;

	virtual ReturnValue_t performOperation();

	virtual ReturnValue_t setHealth(HealthState health);

	virtual HasHealthIF::HealthState getHealth();
protected:
	const uint32_t parentId;

	Mode_t mode;

	Submode_t submode;

	MessageQueue commandQueue;

	ModeHelper modeHelper;

	HealthHelper healthHelper;

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
};

#endif /* CONTROLLERBASE_H_ */

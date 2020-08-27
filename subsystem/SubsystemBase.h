#ifndef SUBSYSTEMBASE_H_
#define SUBSYSTEMBASE_H_

#include "../container/HybridIterator.h"
#include "../health/HasHealthIF.h"
#include "../health/HealthHelper.h"
#include "../modes/HasModesIF.h"
#include "../objectmanager/SystemObject.h"
#include "../returnvalues/HasReturnvaluesIF.h"
#include "modes/HasModeSequenceIF.h"
#include "../tasks/ExecutableObjectIF.h"
#include "../ipc/MessageQueueIF.h"
#include <map>

class SubsystemBase: public SystemObject,
		public HasModesIF,
		public HasHealthIF,
		public HasReturnvaluesIF,
		public ExecutableObjectIF {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::SUBSYSTEM_BASE;
	static const ReturnValue_t CHILD_NOT_FOUND = MAKE_RETURN_CODE(0x01);
	static const ReturnValue_t CHILD_INFO_UPDATED = MAKE_RETURN_CODE(0x02);
	static const ReturnValue_t CHILD_DOESNT_HAVE_MODES = MAKE_RETURN_CODE(0x03);
	static const ReturnValue_t COULD_NOT_INSERT_CHILD = MAKE_RETURN_CODE(0x04);
	static const ReturnValue_t TABLE_CONTAINS_INVALID_OBJECT_ID =
			MAKE_RETURN_CODE(0x05);

	SubsystemBase(object_id_t setObjectId, object_id_t parent,
			Mode_t initialMode = 0, uint16_t commandQueueDepth = 8);
	virtual ~SubsystemBase();

	virtual MessageQueueId_t getCommandQueue() const;

	ReturnValue_t registerChild(object_id_t objectId);

	virtual ReturnValue_t initialize();

	virtual ReturnValue_t performOperation(uint8_t opCode);

	virtual ReturnValue_t setHealth(HealthState health);

	virtual HasHealthIF::HealthState getHealth();

protected:
	struct ChildInfo {
		MessageQueueId_t commandQueue;
		Mode_t mode;
		Submode_t submode;bool healthChanged;
	};

	Mode_t mode;

	Submode_t submode;

	bool childrenChangedMode;

	/**
	 * Always check this against <=0, so you are robust against too many replies
	 */
	int32_t commandsOutstanding;

	MessageQueueIF* commandQueue;

	HealthHelper healthHelper;

	ModeHelper modeHelper;

	const object_id_t parentId;

	typedef std::map<object_id_t, ChildInfo> ChildrenMap;
	ChildrenMap childrenMap;

	void checkCommandQueue();

	/**
	 * We need to know the target Submode, as children are able to inherit the submode
	 */
	ReturnValue_t checkStateAgainstTable(
			HybridIterator<ModeListEntry> tableIter, Submode_t targetSubmode);

	/**
	 * We need to know the target Submode, as children are able to inherit the submode
	 * Still, we have a default for all child implementations which do not use submode inheritance
	 */
	void executeTable(HybridIterator<ModeListEntry> tableIter,
			Submode_t targetSubmode = SUBMODE_NONE);

	ReturnValue_t updateChildMode(MessageQueueId_t queue, Mode_t mode,
			Submode_t submode);

	ReturnValue_t updateChildChangedHealth(MessageQueueId_t queue,
			bool changedHealth = true);

	virtual ReturnValue_t handleModeReply(CommandMessage *message);

	void commandAllChildren(CommandMessage *message);

	ReturnValue_t checkTable(HybridIterator<ModeListEntry> tableIter);

	void replyToCommand(CommandMessage *message);

	void setMode(Mode_t newMode, Submode_t newSubmode);

	void setMode(Mode_t newMode);

	virtual ReturnValue_t handleCommandMessage(CommandMessage *message) = 0;

	virtual void performChildOperation() = 0;

	virtual ReturnValue_t checkModeCommand(Mode_t mode, Submode_t submode,
			uint32_t *msToReachTheMode) = 0;

	virtual void startTransition(Mode_t mode, Submode_t submode) = 0;

	virtual void getMode(Mode_t *mode, Submode_t *submode);

	virtual void setToExternalControl();

	virtual void announceMode(bool recursive);

	virtual void modeChanged();
};

#endif /* SUBSYSTEMBASE_H_ */

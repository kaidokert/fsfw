#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/subsystem/SubsystemBase.h>

SubsystemBase::~SubsystemBase() {

}

ReturnValue_t SubsystemBase::registerChild(object_id_t objectId) {
	ChildInfo info;

	HasModesIF *child = objectManager->get<HasModesIF>(objectId);
	//This is a rather ugly hack to have the changedHealth info for all children available. (needed for FOGs).
	HasHealthIF* healthChild = objectManager->get<HasHealthIF>(objectId);
	if (child == NULL) {
		if (healthChild == NULL) {
			return CHILD_DOESNT_HAVE_MODES;
		} else {
			info.commandQueue = healthChild->getCommandQueue();
			info.mode = MODE_OFF;
		}
	} else {
		info.commandQueue = child->getCommandQueue();
		info.mode = -1;	//intentional to force an initial command during system startup
	}

	info.submode = SUBMODE_NONE;
	info.healthChanged = false;

	std::pair<std::map<object_id_t, ChildInfo>::iterator, bool> returnValue =
			childrenMap.insert(
					std::pair<object_id_t, ChildInfo>(objectId, info));
	if (!(returnValue.second)) {
		return COULD_NOT_INSERT_CHILD;
	} else {
		return RETURN_OK;
	}
}

ReturnValue_t SubsystemBase::checkStateAgainstTable(
		HybridIterator<ModeListEntry> tableIter) {

	std::map<object_id_t, ChildInfo>::iterator childIter;

	for (; tableIter.value != NULL; ++tableIter) {
		object_id_t object = tableIter.value->getObject();

		if ((childIter = childrenMap.find(object)) == childrenMap.end()) {
			return RETURN_FAILED;
		}

		if (childIter->second.mode != tableIter.value->getMode()) {
			return RETURN_FAILED;
		}
		if (childIter->second.submode != tableIter.value->getSubmode()) {
			return RETURN_FAILED;
		}
	}
	return RETURN_OK;
}

void SubsystemBase::executeTable(HybridIterator<ModeListEntry> tableIter) {

	CommandMessage message;

	std::map<object_id_t, ChildInfo>::iterator iter;

	commandsOutstanding = 0;

	for (; tableIter.value != NULL; ++tableIter) {
		object_id_t object = tableIter.value->getObject();
		if ((iter = childrenMap.find(object)) == childrenMap.end()) {
			//illegal table entry
			//TODO: software error
			continue;
		}

		if (healthHelper.healthTable->hasHealth(object)) {
			if (healthHelper.healthTable->isFaulty(object)) {
				ModeMessage::setModeMessage(&message,
						ModeMessage::CMD_MODE_COMMAND, HasModesIF::MODE_OFF,
						SUBMODE_NONE);
			} else {
				if (modeHelper.isForced()) {
					ModeMessage::setModeMessage(&message,
							ModeMessage::CMD_MODE_COMMAND_FORCED,
							tableIter.value->getMode(),
							tableIter.value->getSubmode());
				} else {
					if (healthHelper.healthTable->isCommandable(object)) {
						ModeMessage::setModeMessage(&message,
								ModeMessage::CMD_MODE_COMMAND,
								tableIter.value->getMode(),
								tableIter.value->getSubmode());
					} else {
						continue;
					}
				}
			}
		} else {
			ModeMessage::setModeMessage(&message, ModeMessage::CMD_MODE_COMMAND,
					tableIter.value->getMode(), tableIter.value->getSubmode());
		}
		//TODO: This may causes trouble with more than two layers, sys commands subsys off, which is already off, but children are on (external).
		// So, they stay on. Might only be an issue if mode is forced, so we do
		if ((iter->second.mode == ModeMessage::getMode(&message))
				&& (iter->second.submode == ModeMessage::getSubmode(&message)) && !modeHelper.isForced()) {
			continue; //don't send redundant mode commands (produces event spam)
		}
		ReturnValue_t result = commandQueue.sendMessage(
				iter->second.commandQueue, &message);
		if (result != RETURN_OK) {
			//TODO OBSW internal error
		}
		++commandsOutstanding;
	}

}

ReturnValue_t SubsystemBase::updateChildMode(MessageQueueId_t queue,
		Mode_t mode, Submode_t submode) {
	std::map<object_id_t, ChildInfo>::iterator iter;

	for (iter = childrenMap.begin(); iter != childrenMap.end(); iter++) {
		if (iter->second.commandQueue == queue) {
			iter->second.mode = mode;
			iter->second.submode = submode;
			return RETURN_OK;
		}
	}
	return CHILD_NOT_FOUND;
}

ReturnValue_t SubsystemBase::updateChildChangedHealth(MessageQueueId_t queue,
		bool changedHealth) {
	for (auto iter = childrenMap.begin(); iter != childrenMap.end(); iter++) {
		if (iter->second.commandQueue == queue) {
			iter->second.healthChanged = changedHealth;
			return RETURN_OK;
		}
	}
	return CHILD_NOT_FOUND;
}

SubsystemBase::SubsystemBase(object_id_t setObjectId, object_id_t parent,
		Mode_t initialMode, uint16_t commandQueueDepth) :
		SystemObject(setObjectId), mode(initialMode), submode(SUBMODE_NONE), childrenChangedMode(
				false), commandsOutstanding(0), commandQueue(commandQueueDepth,
				CommandMessage::MAX_MESSAGE_SIZE), healthHelper(this, setObjectId), modeHelper(this), parentId(parent) {
}

MessageQueueId_t SubsystemBase::getCommandQueue() const {
	return commandQueue.getId();
}

ReturnValue_t SubsystemBase::initialize() {
	MessageQueueId_t parentQueue = 0;
	ReturnValue_t result = SystemObject::initialize();

	if (result != RETURN_OK) {
		return result;
	}

	if (parentId != 0) {
		SubsystemBase *parent = objectManager->get<SubsystemBase>(parentId);
		if (parent == NULL) {
			return RETURN_FAILED;
		}
		parentQueue = parent->getCommandQueue();

		parent->registerChild(getObjectId());
	}

	result = healthHelper.initialize(parentQueue);

	if (result != RETURN_OK) {
		return result;
	}

	result = modeHelper.initialize(parentQueue);

	if (result != RETURN_OK) {
		return result;
	}

	return RETURN_OK;
}

ReturnValue_t SubsystemBase::performOperation() {

	childrenChangedMode = false;

	checkCommandQueue();

	performChildOperation();

	return RETURN_OK;
}

ReturnValue_t SubsystemBase::handleModeReply(CommandMessage* message) {
	switch (message->getCommand()) {
	case ModeMessage::REPLY_MODE_INFO:
		updateChildMode(message->getSender(), ModeMessage::getMode(message),
				ModeMessage::getSubmode(message));
		childrenChangedMode = true;
		return RETURN_OK;
	case ModeMessage::REPLY_MODE_REPLY:
	case ModeMessage::REPLY_WRONG_MODE_REPLY:
		updateChildMode(message->getSender(), ModeMessage::getMode(message),
				ModeMessage::getSubmode(message));
		childrenChangedMode = true;
		commandsOutstanding--;
		return RETURN_OK;
	case ModeMessage::REPLY_CANT_REACH_MODE:
		commandsOutstanding--;
		{
			for (auto iter = childrenMap.begin(); iter != childrenMap.end();
					iter++) {
				if (iter->second.commandQueue == message->getSender()) {
					triggerEvent(MODE_CMD_REJECTED, iter->first, message->getParameter());
				}
			}
		}
		return RETURN_OK;
//	case ModeMessage::CMD_MODE_COMMAND:
//		handleCommandedMode(message);
//		return RETURN_OK;
//	case ModeMessage::CMD_MODE_ANNOUNCE:
//		triggerEvent(MODE_INFO, mode, submode);
//		return RETURN_OK;
//	case ModeMessage::CMD_MODE_ANNOUNCE_RECURSIVELY:
//		triggerEvent(MODE_INFO, mode, submode);
//		commandAllChildren(message);
//		return RETURN_OK;
	default:
		return RETURN_FAILED;
	}
}

ReturnValue_t SubsystemBase::checkTable(
		HybridIterator<ModeListEntry> tableIter) {
	for (; tableIter.value != NULL; ++tableIter) {
		if (childrenMap.find(tableIter.value->getObject())
				== childrenMap.end()) {
			return TABLE_CONTAINS_INVALID_OBJECT_ID;
		}
	}
	return RETURN_OK;
}

void SubsystemBase::replyToCommand(CommandMessage* message) {
	commandQueue.reply(message);
}

void SubsystemBase::setMode(Mode_t newMode, Submode_t newSubmode) {
	modeHelper.modeChanged(newMode, newSubmode);
	mode = newMode;
	submode = newSubmode;
	announceMode(false);
}

void SubsystemBase::setMode(Mode_t newMode) {
	setMode(newMode, submode);
}

void SubsystemBase::commandAllChildren(CommandMessage* message) {
	std::map<object_id_t, ChildInfo>::iterator iter;
	for (iter = childrenMap.begin(); iter != childrenMap.end(); ++iter) {
		commandQueue.sendMessage(iter->second.commandQueue, message);
	}
}

void SubsystemBase::getMode(Mode_t* mode, Submode_t* submode) {
	*mode = this->mode;
	*submode = this->submode;
}

void SubsystemBase::setToExternalControl() {
	healthHelper.setHealth(EXTERNAL_CONTROL);
}

void SubsystemBase::announceMode(bool recursive) {
	triggerEvent(MODE_INFO, mode, submode);
	if (recursive) {
		CommandMessage command;
		ModeMessage::setModeMessage(&command,
				ModeMessage::CMD_MODE_ANNOUNCE_RECURSIVELY, 0, 0);
		commandAllChildren(&command);
	}
}

void SubsystemBase::checkCommandQueue() {
	ReturnValue_t result;
	CommandMessage message;

	for (result = commandQueue.receiveMessage(&message); result == RETURN_OK;
			result = commandQueue.receiveMessage(&message)) {

		result = healthHelper.handleHealthCommand(&message);
		if (result == RETURN_OK) {
			continue;
		}

		result = modeHelper.handleModeCommand(&message);
		if (result == RETURN_OK) {
			continue;
		}

		result = handleModeReply(&message);
		if (result == RETURN_OK) {
			continue;
		}

		result = handleCommandMessage(&message);
		if (result != RETURN_OK) {
			CommandMessage reply;
			reply.setReplyRejected(CommandMessage::UNKNOW_COMMAND, message.getCommand());
			replyToCommand(&reply);
		}
	}
}

ReturnValue_t SubsystemBase::setHealth(HealthState health) {
	switch (health) {
	case HEALTHY:
	case EXTERNAL_CONTROL:
		healthHelper.setHealth(health);
		return RETURN_OK;
	default:
		return INVALID_HEALTH_STATE;
	}
}

HasHealthIF::HealthState SubsystemBase::getHealth() {
	return healthHelper.getHealth();
}

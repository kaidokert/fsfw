#include <framework/modes/HasModesIF.h>
#include <framework/modes/ModeHelper.h>
#include <framework/ipc/MessageQueueSenderIF.h>

ModeHelper::ModeHelper(HasModesIF *owner) :
		theOneWhoCommandedAMode(0), commandedMode(HasModesIF::MODE_OFF), commandedSubmode(
				HasModesIF::SUBMODE_NONE), owner(owner), parentQueueId(0), forced(
				false) {
}

ModeHelper::~ModeHelper() {

}

ReturnValue_t ModeHelper::handleModeCommand(CommandMessage* message) {
	CommandMessage reply;
	Mode_t mode;
	Submode_t submode;
	switch (message->getCommand()) {
	case ModeMessage::CMD_MODE_COMMAND_FORCED:
		forced = true;
		/* NO BREAK falls through*/
	case ModeMessage::CMD_MODE_COMMAND: {
		mode = ModeMessage::getMode(message);
		submode = ModeMessage::getSubmode(message);
		uint32_t timeout;
		ReturnValue_t result = owner->checkModeCommand(mode, submode, &timeout);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			ModeMessage::cantReachMode(&reply, result);
			MessageQueueSenderIF::sendMessage(message->getSender(), &reply,
					owner->getCommandQueue());
			break;
		}
		//Free to start transition
		theOneWhoCommandedAMode = message->getSender();
		commandedMode = mode;
		commandedSubmode = submode;

		if ((parentQueueId != MessageQueueSenderIF::NO_QUEUE)
				&& (theOneWhoCommandedAMode != parentQueueId)) {
			owner->setToExternalControl();
		}

		countdown.setTimeout(timeout);

		owner->startTransition(mode, submode);
	}
		break;
	case ModeMessage::CMD_MODE_READ: {
		owner->getMode(&mode, &submode);
		ModeMessage::setModeMessage(&reply, ModeMessage::REPLY_MODE_REPLY, mode,
				submode);
		MessageQueueSenderIF::sendMessage(message->getSender(), &reply,
				owner->getCommandQueue());
	}
		break;
	case ModeMessage::CMD_MODE_ANNOUNCE:
		owner->announceMode(false);
		break;
	case ModeMessage::CMD_MODE_ANNOUNCE_RECURSIVELY:
		owner->announceMode(true);
		break;
	default:
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t ModeHelper::initialize(MessageQueueId_t parentQueueId) {
	setParentQueue(parentQueueId);
	return initialize();
}

void ModeHelper::modeChanged(Mode_t mode, Submode_t submode) {
	forced = false;
	CommandMessage reply;
	if (theOneWhoCommandedAMode != MessageQueueSenderIF::NO_QUEUE) {
		if ((mode != commandedMode) || (submode != commandedSubmode)) {
			ModeMessage::setModeMessage(&reply,
					ModeMessage::REPLY_WRONG_MODE_REPLY, mode, submode);
		} else {
			ModeMessage::setModeMessage(&reply, ModeMessage::REPLY_MODE_REPLY,
					mode, submode);
		}
		MessageQueueSenderIF::sendMessage(theOneWhoCommandedAMode, &reply,
				owner->getCommandQueue());
	}
	if (theOneWhoCommandedAMode != parentQueueId
			&& parentQueueId != MessageQueueSenderIF::NO_QUEUE) {
		ModeMessage::setModeMessage(&reply, ModeMessage::REPLY_MODE_INFO, mode,
				submode);
		MessageQueueSenderIF::sendMessage(parentQueueId, &reply, owner->getCommandQueue());
	}
	theOneWhoCommandedAMode = MessageQueueSenderIF::NO_QUEUE;
}

void ModeHelper::startTimer(uint32_t timeoutMs) {
	countdown.setTimeout(timeoutMs);
}

void ModeHelper::setParentQueue(MessageQueueId_t parentQueueId) {
	this->parentQueueId = parentQueueId;
}

ReturnValue_t ModeHelper::initialize(void) {
	return HasReturnvaluesIF::RETURN_OK;
}

bool ModeHelper::isTimedOut() {
	return countdown.hasTimedOut();
}

bool ModeHelper::isForced() {
	return forced;
}

void ModeHelper::setForced(bool forced) {
	this->forced = forced;
}

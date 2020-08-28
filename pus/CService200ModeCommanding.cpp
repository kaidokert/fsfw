#include "../pus/CService200ModeCommanding.h"
#include "../pus/servicepackets/Service200Packets.h"

#include "../modes/HasModesIF.h"
#include "../serviceinterface/ServiceInterfaceStream.h"
#include "../serialize/SerialLinkedListAdapter.h"
#include "../modes/ModeMessage.h"

CService200ModeCommanding::CService200ModeCommanding(object_id_t objectId,
        uint16_t apid, uint8_t serviceId):
        CommandingServiceBase(objectId, apid, serviceId,
	    NUMBER_OF_PARALLEL_COMMANDS,COMMAND_TIMEOUT_SECONDS) {}

CService200ModeCommanding::~CService200ModeCommanding() {}

ReturnValue_t CService200ModeCommanding::isValidSubservice(uint8_t subservice) {
	switch(subservice) {
	case(Subservice::COMMAND_MODE_COMMAND):
	case(Subservice::COMMAND_MODE_READ):
	case(Subservice::COMMAND_MODE_ANNCOUNCE):
		return RETURN_OK;
	default:
		return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
	}
}


ReturnValue_t CService200ModeCommanding::getMessageQueueAndObject(
		uint8_t subservice, const uint8_t *tcData, size_t tcDataLen,
		MessageQueueId_t *id, object_id_t *objectId) {
    if(tcDataLen < sizeof(object_id_t)) {
        return CommandingServiceBase::INVALID_TC;
    }
    SerializeAdapter::deSerialize(objectId, &tcData, &tcDataLen,
            SerializeIF::Endianness::BIG);

	return checkInterfaceAndAcquireMessageQueue(id,objectId);
}

ReturnValue_t CService200ModeCommanding::checkInterfaceAndAcquireMessageQueue(
        MessageQueueId_t* messageQueueToSet, object_id_t* objectId) {
    HasModesIF * destination = objectManager->get<HasModesIF>(*objectId);
    if(destination == nullptr) {
        return CommandingServiceBase::INVALID_OBJECT;

    }

    *messageQueueToSet = destination->getCommandQueue();
    return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t CService200ModeCommanding::prepareCommand(
		CommandMessage* message,uint8_t subservice, const uint8_t *tcData,
		size_t tcDataLen, uint32_t *state, object_id_t objectId) {
	ModePacket modeCommandPacket;
	ReturnValue_t result = modeCommandPacket.deSerialize(&tcData,
			&tcDataLen, SerializeIF::Endianness::BIG);
	if (result != RETURN_OK) {
		return result;
	}

	ModeMessage::setModeMessage(dynamic_cast<CommandMessage*>(message),
	        ModeMessage::CMD_MODE_COMMAND, modeCommandPacket.getMode(),
	        modeCommandPacket.getSubmode());
	return result;
}


ReturnValue_t CService200ModeCommanding::handleReply(
		const CommandMessage* reply, Command_t previousCommand,
		uint32_t *state, CommandMessage* optionalNextCommand,
		object_id_t objectId, bool *isStep) {
	Command_t replyId = reply->getCommand();
	ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
	switch(replyId) {
	case(ModeMessage::REPLY_MODE_REPLY): {
		result = prepareModeReply(reply, objectId);
		break;
	}
	case(ModeMessage::REPLY_WRONG_MODE_REPLY): {
		result = prepareWrongModeReply(reply, objectId);
		break;
	}
	case(ModeMessage::REPLY_CANT_REACH_MODE): {
		result = prepareCantReachModeReply(reply, objectId);
		break;
	}
	case(ModeMessage::REPLY_MODE_INFO):
		result = INVALID_REPLY;
		break;
	default:
		result = RETURN_FAILED;
	}
	return result;
}

ReturnValue_t CService200ModeCommanding::prepareModeReply(
		const CommandMessage *reply, object_id_t objectId) {
	ModePacket modeReplyPacket(objectId,
			ModeMessage::getMode(reply),
			ModeMessage::getSubmode(reply));
	return sendTmPacket(Subservice::REPLY_MODE_REPLY, &modeReplyPacket);
}

ReturnValue_t CService200ModeCommanding::prepareWrongModeReply(
		const CommandMessage *reply, object_id_t objectId) {
	ModePacket wrongModeReply(objectId, ModeMessage::getMode(reply),
			ModeMessage::getSubmode(reply));
	return sendTmPacket(Subservice::REPLY_WRONG_MODE_REPLY, &wrongModeReply);
}

ReturnValue_t CService200ModeCommanding::prepareCantReachModeReply(
		const CommandMessage *reply, object_id_t objectId) {
	CantReachModePacket cantReachModePacket(objectId,
	        ModeMessage::getCantReachModeReason(reply));
	return sendTmPacket(Subservice::REPLY_CANT_REACH_MODE,
	        &cantReachModePacket);
}

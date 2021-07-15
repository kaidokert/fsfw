#include "fsfw/pus/CService201HealthCommanding.h"
#include "fsfw/pus/servicepackets/Service201Packets.h"

#include "fsfw/health/HasHealthIF.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/health/HealthMessage.h"


CService201HealthCommanding::CService201HealthCommanding(object_id_t objectId,
        uint16_t apid, uint8_t serviceId, uint8_t numParallelCommands,
        uint16_t commandTimeoutSeconds):
		CommandingServiceBase(objectId, apid, serviceId,
		        numParallelCommands, commandTimeoutSeconds) {
}

CService201HealthCommanding::~CService201HealthCommanding() {
}

ReturnValue_t CService201HealthCommanding::isValidSubservice(uint8_t subservice) {
	switch(subservice) {
	case(Subservice::COMMAND_SET_HEALTH):
	case(Subservice::COMMAND_ANNOUNCE_HEALTH):
	case(Subservice::COMMAND_ANNOUNCE_HEALTH_ALL):
		return RETURN_OK;
	default:
#if FSFW_CPP_OSTREAM_ENABLED == 1
		sif::error << "Invalid Subservice" << std::endl;
#endif
		return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
	}
}

ReturnValue_t CService201HealthCommanding::getMessageQueueAndObject(
        uint8_t subservice, const uint8_t *tcData, size_t tcDataLen,
        MessageQueueId_t *id, object_id_t *objectId) {
    if(tcDataLen < sizeof(object_id_t)) {
        return CommandingServiceBase::INVALID_TC;
    }
    SerializeAdapter::deSerialize(objectId, &tcData, &tcDataLen,
            SerializeIF::Endianness::BIG);

    return checkInterfaceAndAcquireMessageQueue(id,objectId);
}

ReturnValue_t CService201HealthCommanding::checkInterfaceAndAcquireMessageQueue(
        MessageQueueId_t* messageQueueToSet, object_id_t* objectId) {
    HasHealthIF * destination = ObjectManager::instance()->get<HasHealthIF>(*objectId);
    if(destination == nullptr) {
        return CommandingServiceBase::INVALID_OBJECT;
    }

    *messageQueueToSet = destination->getCommandQueue();
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t CService201HealthCommanding::prepareCommand(
        CommandMessage* message, uint8_t subservice, const uint8_t *tcData,
		size_t tcDataLen, uint32_t *state, object_id_t objectId) {
    ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
    switch(subservice) {
    case(Subservice::COMMAND_SET_HEALTH): {
        HealthSetCommand healthCommand;
        result = healthCommand.deSerialize(&tcData, &tcDataLen,
                SerializeIF::Endianness::BIG);
        if (result != RETURN_OK) {
            break;
        }
        HealthMessage::setHealthMessage(message, HealthMessage::HEALTH_SET,
                healthCommand.getHealth());
        break;
    }
    case(Subservice::COMMAND_ANNOUNCE_HEALTH): {
        HealthMessage::setHealthMessage(message,
                HealthMessage::HEALTH_ANNOUNCE);
        break;
    }
    case(Subservice::COMMAND_ANNOUNCE_HEALTH_ALL): {
        HealthMessage::setHealthMessage(message,
                HealthMessage::HEALTH_ANNOUNCE_ALL);
        break;
    }
    }
    return result;
}

ReturnValue_t CService201HealthCommanding::handleReply
		(const CommandMessage* reply, Command_t previousCommand,
		uint32_t *state, CommandMessage* optionalNextCommand,
		object_id_t objectId, bool *isStep) {
	Command_t replyId = reply->getCommand();
	if (replyId == HealthMessage::REPLY_HEALTH_SET) {
		return EXECUTION_COMPLETE;
	}
	else if(replyId == CommandMessageIF::REPLY_REJECTED) {
		return reply->getReplyRejectedReason();
	}
	return CommandingServiceBase::INVALID_REPLY;
}

// Not used for now, health state already reported by event
ReturnValue_t CService201HealthCommanding::prepareHealthSetReply(
        const CommandMessage* reply) {
	prepareHealthSetReply(reply);
	uint8_t health = static_cast<uint8_t>(HealthMessage::getHealth(reply));
	uint8_t oldHealth = static_cast<uint8_t>(HealthMessage::getOldHealth(reply));
	HealthSetReply healthSetReply(health, oldHealth);
	return sendTmPacket(Subservice::REPLY_HEALTH_SET, &healthSetReply);
}


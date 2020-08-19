#include "CService201HealthCommanding.h"

#include "../health/HasHealthIF.h"
#include "../serviceinterface/ServiceInterfaceStream.h"
#include "../health/HealthMessage.h"
#include "servicepackets/Service201Packets.h"

CService201HealthCommanding::CService201HealthCommanding(object_id_t objectId,
        uint16_t apid, uint8_t serviceId):
	CommandingServiceBase(objectId, apid, serviceId,
			NUMBER_OF_PARALLEL_COMMANDS,COMMAND_TIMEOUT_SECONDS) {
}

CService201HealthCommanding::~CService201HealthCommanding() {
}

ReturnValue_t CService201HealthCommanding::isValidSubservice(uint8_t subservice) {
	switch(subservice) {
	case(Subservice::COMMAND_SET_HEALTH):
		return RETURN_OK;
	default:
		sif::error << "Invalid Subservice" << std::endl;
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
    HasHealthIF * destination = objectManager->get<HasHealthIF>(*objectId);
    if(destination == nullptr) {
        return CommandingServiceBase::INVALID_OBJECT;
    }

    *messageQueueToSet = destination->getCommandQueue();
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t CService201HealthCommanding::prepareCommand
		(CommandMessage* message, uint8_t subservice, const uint8_t *tcData,
		size_t tcDataLen, uint32_t *state, object_id_t objectId) {
	HealthCommand healthCommand;
	ReturnValue_t result = healthCommand.deSerialize(&tcData, &tcDataLen,
	        SerializeIF::Endianness::BIG);
	if (result != RETURN_OK) {
		return result;
	} else {
		HealthMessage::setHealthMessage(dynamic_cast<CommandMessage*>(message),
				HealthMessage::HEALTH_SET, healthCommand.getHealth());
		return result;
	}
}

ReturnValue_t CService201HealthCommanding::handleReply
		(const CommandMessage* reply, Command_t previousCommand,
		uint32_t *state, CommandMessage* optionalNextCommand,
		object_id_t objectId, bool *isStep) {
	Command_t replyId = reply->getCommand();
	if (replyId == HealthMessage::REPLY_HEALTH_SET) {
		prepareHealthSetReply(reply);
	}
	return RETURN_OK;
}

void CService201HealthCommanding::prepareHealthSetReply(
        const CommandMessage* reply) {
	prepareHealthSetReply(reply);
	uint8_t health = static_cast<uint8_t>(HealthMessage::getHealth(reply));
	uint8_t oldHealth = static_cast<uint8_t>(HealthMessage::getOldHealth(reply));
	HealthSetReply healthSetReply(health, oldHealth);
	sendTmPacket(Subservice::REPLY_HEALTH_SET,&healthSetReply);
}


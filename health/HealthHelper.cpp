#include <framework/health/HealthHelper.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

HealthHelper::HealthHelper(HasHealthIF* owner, object_id_t objectId) :
		objectId(objectId), owner(owner) {
}

HealthHelper::~HealthHelper() {
}

ReturnValue_t HealthHelper::handleHealthCommand(CommandMessage* message) {
	switch (message->getCommand()) {
	case HealthMessage::HEALTH_SET:
		handleSetHealthCommand(message);
		return HasReturnvaluesIF::RETURN_OK;
	case HealthMessage::HEALTH_ANNOUNCE: {
		eventSender->forwardEvent(HasHealthIF::HEALTH_INFO, getHealth(),
				getHealth());
	}
		return HasReturnvaluesIF::RETURN_OK;
	default:
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

HasHealthIF::HealthState HealthHelper::getHealth() {
	return healthTable->getHealth(objectId);
}

ReturnValue_t HealthHelper::initialize(MessageQueueId_t parentQueue) {
	setParentQeueue(parentQueue);
	return initialize();
}

void HealthHelper::setParentQeueue(MessageQueueId_t parentQueue) {
	this->parentQueue = parentQueue;
}

ReturnValue_t HealthHelper::initialize() {
	healthTable = objectManager->get<HealthTableIF>(objects::HEALTH_TABLE);
	eventSender = objectManager->get<EventReportingProxyIF>(objectId);
	// TODO: Better returnvalues
	if ((healthTable == NULL) || eventSender == NULL) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	ReturnValue_t result = healthTable->registerObject(objectId,
			HasHealthIF::HEALTHY);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

void HealthHelper::setHealth(HasHealthIF::HealthState health) {
	HasHealthIF::HealthState oldHealth = getHealth();
	eventSender->forwardEvent(HasHealthIF::HEALTH_INFO, health, oldHealth);
	if (health != oldHealth) {
		healthTable->setHealth(objectId, health);
		informParent(health, oldHealth);
	}
}

void HealthHelper::informParent(HasHealthIF::HealthState health,
		HasHealthIF::HealthState oldHealth) {
	if (parentQueue == MessageQueueMessageIF::NO_QUEUE) {
		return;
	}
	CommandMessage information;
	HealthMessage::setHealthMessage(&information, HealthMessage::HEALTH_INFO,
			health, oldHealth);
	if (MessageQueueSenderIF::sendMessage(parentQueue, &information,
	        owner->getCommandQueue()) != HasReturnvaluesIF::RETURN_OK) {
		sif::debug << "HealthHelper::informParent: sending health reply failed."
				<< std::endl;
	}
}

void HealthHelper::handleSetHealthCommand(CommandMessage* command) {
	ReturnValue_t result = owner->setHealth(HealthMessage::getHealth(command));
	if (command->getSender() == MessageQueueMessageIF::NO_QUEUE) {
		return;
	}
	CommandMessage reply;
	if (result == HasReturnvaluesIF::RETURN_OK) {
		HealthMessage::setHealthMessage(&reply,
				HealthMessage::REPLY_HEALTH_SET);
	} else {
		reply.setReplyRejected(result, command->getCommand());
	}
	if (MessageQueueSenderIF::sendMessage(command->getSender(), &reply,
	        owner->getCommandQueue()) != HasReturnvaluesIF::RETURN_OK) {
		sif::debug << "HealthHelper::handleHealthCommand: sending health "
		        "reply failed." << std::endl;

	}
}

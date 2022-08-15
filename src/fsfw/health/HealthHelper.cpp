#include "fsfw/health/HealthHelper.h"

#include "fsfw/serviceinterface/ServiceInterface.h"

HealthHelper::HealthHelper(HasHealthIF* owner, object_id_t objectId)
    : objectId(objectId), owner(owner) {}

HealthHelper::~HealthHelper() { healthTable->removeObject(objectId); }

ReturnValue_t HealthHelper::handleHealthCommand(CommandMessage* message) {
  switch (message->getCommand()) {
    case HealthMessage::HEALTH_SET:
      handleSetHealthCommand(message);
      return returnvalue::OK;
    case HealthMessage::HEALTH_ANNOUNCE: {
      eventSender->forwardEvent(HasHealthIF::HEALTH_INFO, getHealth(), getHealth());
    }
      return returnvalue::OK;
    default:
      return returnvalue::FAILED;
  }
}

HasHealthIF::HealthState HealthHelper::getHealth() { return healthTable->getHealth(objectId); }

ReturnValue_t HealthHelper::initialize(MessageQueueId_t parentQueue) {
  setParentQueue(parentQueue);
  return initialize();
}

void HealthHelper::setParentQueue(MessageQueueId_t parentQueue) { this->parentQueue = parentQueue; }

ReturnValue_t HealthHelper::initialize() {
  healthTable = ObjectManager::instance()->get<HealthTableIF>(objects::HEALTH_TABLE);
  eventSender = ObjectManager::instance()->get<EventReportingProxyIF>(objectId);

  if (healthTable == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "HealthHelper::initialize: Health table object needs"
                  "to be created in factory."
               << std::endl;
#endif
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }

  if (eventSender == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "HealthHelper::initialize: Owner has to implement "
                  "ReportingProxyIF."
               << std::endl;
#endif
    return ObjectManagerIF::CHILD_INIT_FAILED;
  }

  ReturnValue_t result = healthTable->registerObject(objectId, HasHealthIF::HEALTHY);
  if (result != returnvalue::OK) {
    return result;
  }
  return returnvalue::OK;
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
  if (parentQueue == MessageQueueIF::NO_QUEUE) {
    return;
  }
  CommandMessage information;
  HealthMessage::setHealthMessage(&information, HealthMessage::HEALTH_INFO, health, oldHealth);
  if (MessageQueueSenderIF::sendMessage(parentQueue, &information, owner->getCommandQueue()) !=
      returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "HealthHelper::informParent: sending health reply failed." << std::endl;
#endif
  }
}

void HealthHelper::handleSetHealthCommand(CommandMessage* command) {
  ReturnValue_t result = owner->setHealth(HealthMessage::getHealth(command));
  if (command->getSender() == MessageQueueIF::NO_QUEUE) {
    return;
  }
  CommandMessage reply;
  if (result == returnvalue::OK) {
    HealthMessage::setHealthMessage(&reply, HealthMessage::REPLY_HEALTH_SET);
  } else {
    reply.setReplyRejected(result, command->getCommand());
  }
  if (MessageQueueSenderIF::sendMessage(command->getSender(), &reply, owner->getCommandQueue()) !=
      returnvalue::OK) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::debug << "HealthHelper::handleHealthCommand: sending health "
                  "reply failed."
               << std::endl;
#endif
  }
}

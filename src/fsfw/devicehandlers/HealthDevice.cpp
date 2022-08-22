#include "fsfw/devicehandlers/HealthDevice.h"

#include "fsfw/ipc/QueueFactory.h"

HealthDevice::HealthDevice(object_id_t setObjectId, MessageQueueId_t parentQueue)
    : SystemObject(setObjectId),
      lastHealth(HEALTHY),
      parentQueue(parentQueue),
      commandQueue(),
      healthHelper(this, setObjectId) {
  commandQueue = QueueFactory::instance()->createMessageQueue(3);
}

HealthDevice::~HealthDevice() { QueueFactory::instance()->deleteMessageQueue(commandQueue); }

ReturnValue_t HealthDevice::performOperation(uint8_t opCode) {
  CommandMessage command;
  ReturnValue_t result = commandQueue->receiveMessage(&command);
  if (result == returnvalue::OK) {
    result = healthHelper.handleHealthCommand(&command);
  }
  return result;
}

ReturnValue_t HealthDevice::initialize() {
  ReturnValue_t result = SystemObject::initialize();
  if (result != returnvalue::OK) {
    return result;
  }
  if (parentQueue != 0) {
    return healthHelper.initialize(parentQueue);
  } else {
    return healthHelper.initialize();
  }
}

MessageQueueId_t HealthDevice::getCommandQueue() const { return commandQueue->getId(); }

void HealthDevice::setParentQueue(MessageQueueId_t parentQueue) {
  healthHelper.setParentQueue(parentQueue);
}

bool HealthDevice::hasHealthChanged() {
  bool changed;
  HealthState currentHealth = healthHelper.getHealth();
  changed = currentHealth != lastHealth;
  lastHealth = currentHealth;
  return changed;
}

ReturnValue_t HealthDevice::setHealth(HealthState health) {
  healthHelper.setHealth(health);
  return returnvalue::OK;
}

HasHealthIF::HealthState HealthDevice::getHealth() { return healthHelper.getHealth(); }

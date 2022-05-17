#include "fsfw/thermal/AbstractTemperatureSensor.h"

#include "fsfw/ipc/QueueFactory.h"

AbstractTemperatureSensor::AbstractTemperatureSensor(object_id_t setObjectid,
                                                     ThermalModuleIF *thermalModule)
    : SystemObject(setObjectid),
      commandQueue(NULL),
      healthHelper(this, setObjectid),
      parameterHelper(this) {
  if (thermalModule != NULL) {
    thermalModule->registerSensor(this);
  }
  commandQueue = QueueFactory::instance()->createMessageQueue();
}

AbstractTemperatureSensor::~AbstractTemperatureSensor() {
  QueueFactory::instance()->deleteMessageQueue(commandQueue);
}

MessageQueueId_t AbstractTemperatureSensor::getCommandQueue() const {
  return commandQueue->getId();
}

ReturnValue_t AbstractTemperatureSensor::initialize() {
  ReturnValue_t result = SystemObject::initialize();
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  result = healthHelper.initialize();
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  result = parameterHelper.initialize();
  return result;
}

ReturnValue_t AbstractTemperatureSensor::performOperation(uint8_t opCode) {
  handleCommandQueue();
  doChildOperation();
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t AbstractTemperatureSensor::performHealthOp() {
  handleCommandQueue();
  return HasReturnvaluesIF::RETURN_OK;
}

void AbstractTemperatureSensor::handleCommandQueue() {
  CommandMessage command;
  ReturnValue_t result = commandQueue->receiveMessage(&command);
  if (result == HasReturnvaluesIF::RETURN_OK) {
    result = healthHelper.handleHealthCommand(&command);
    if (result == HasReturnvaluesIF::RETURN_OK) {
      return;
    }
    result = parameterHelper.handleParameterMessage(&command);
    if (result == HasReturnvaluesIF::RETURN_OK) {
      return;
    }
    command.setToUnknownCommand();
    commandQueue->reply(&command);
  }
}

ReturnValue_t AbstractTemperatureSensor::setHealth(HealthState health) {
  healthHelper.setHealth(health);
  return HasReturnvaluesIF::RETURN_OK;
}

HasHealthIF::HealthState AbstractTemperatureSensor::getHealth() { return healthHelper.getHealth(); }

#include "DeviceHandlerCommander.h"

DeviceHandlerCommander::DeviceHandlerCommander(object_id_t objectId)
    : SystemObject(objectId), commandActionHelper(this) {
  auto mqArgs = MqArgs(this->getObjectId());
  commandQueue = QueueFactory::instance()->createMessageQueue(
      QUEUE_SIZE, MessageQueueMessage::MAX_MESSAGE_SIZE, &mqArgs);
}

DeviceHandlerCommander::~DeviceHandlerCommander() {}

ReturnValue_t DeviceHandlerCommander::performOperation(uint8_t operationCode) {
  readCommandQueue();
}

ReturnValue_t DeviceHandlerCommander::initialize() {
  ReturnValue_t result = commandActionHelper.initialize();
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  result = actionHelper.initialize(commandQueue);
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  return HasReturnvaluesIF::RETURN_OK;
}

MessageQueueIF* DeviceHandlerCommander::getCommandQueuePtr() { return commandQueue; }

void DeviceHandlerCommander::stepSuccessfulReceived(ActionId_t actionId, uint8_t step) {}

void DeviceHandlerCommander::stepFailedReceived(ActionId_t actionId, uint8_t step,
                                                ReturnValue_t returnCode) {}

void DeviceHandlerCommander::dataReceived(ActionId_t actionId, const uint8_t* data, uint32_t size) {
}

void DeviceHandlerCommander::completionSuccessfulReceived(ActionId_t actionId) {
  lastReplyReturnCode = RETURN_OK;
}

void DeviceHandlerCommander::completionFailedReceived(ActionId_t actionId,
                                                      ReturnValue_t returnCode) {
  lastReplyReturnCode = returnCode;
}

void DeviceHandlerCommander::readCommandQueue() {
  CommandMessage message;
  ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
  for (result = commandQueue->receiveMessage(&message); result == HasReturnvaluesIF::RETURN_OK;
       result = commandQueue->receiveMessage(&message)) {
    if (result != HasReturnvaluesIF::RETURN_OK) {
      continue;
    }
    result = commandActionHelper.handleReply(&message);
    if (result == HasReturnvaluesIF::RETURN_OK) {
      continue;
    }
  }
}

ReturnValue_t DeviceHandlerCommander::sendCommand(object_id_t target, ActionId_t actionId) {
  return commandActionHelper.commandAction(target, actionId, nullptr, 0);
}

ReturnValue_t DeviceHandlerCommander::getReplyReturnCode() { return lastReplyReturnCode; }

void DeviceHandlerCommander::resetReplyReturnCode() {
  lastReplyReturnCode = RETURN_FAILED;
}

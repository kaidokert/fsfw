#include "DeviceHandlerCommander.h"

#include <fsfw/ipc/QueueFactory.h>

DeviceHandlerCommander::DeviceHandlerCommander(object_id_t objectId)
    : SystemObject(objectId), commandActionHelper(this) {
  auto mqArgs = MqArgs(this->getObjectId());
  commandQueue = QueueFactory::instance()->createMessageQueue(
      QUEUE_SIZE, MessageQueueMessage::MAX_MESSAGE_SIZE, &mqArgs);
}

DeviceHandlerCommander::~DeviceHandlerCommander() {}

ReturnValue_t DeviceHandlerCommander::performOperation(uint8_t operationCode) {
  readCommandQueue();
  return returnvalue::OK;
}

ReturnValue_t DeviceHandlerCommander::initialize() {
  ReturnValue_t result = commandActionHelper.initialize();
  if (result != returnvalue::OK) {
    return result;
  }
  return returnvalue::OK;
}

MessageQueueIF* DeviceHandlerCommander::getCommandQueuePtr() { return commandQueue; }

void DeviceHandlerCommander::stepSuccessfulReceived(ActionId_t actionId, uint8_t step) {}

void DeviceHandlerCommander::stepFailedReceived(ActionId_t actionId, uint8_t step,
                                                ReturnValue_t returnCode) {}

void DeviceHandlerCommander::dataReceived(ActionId_t actionId, const uint8_t* data, uint32_t size) {
}

void DeviceHandlerCommander::completionSuccessfulReceived(ActionId_t actionId) {
  lastReplyReturnCode = returnvalue::OK;
}

void DeviceHandlerCommander::completionFailedReceived(ActionId_t actionId,
                                                      ReturnValue_t returnCode) {
  lastReplyReturnCode = returnCode;
}

void DeviceHandlerCommander::readCommandQueue() {
  CommandMessage message;
  ReturnValue_t result = returnvalue::OK;
  for (result = commandQueue->receiveMessage(&message); result == returnvalue::OK;
       result = commandQueue->receiveMessage(&message)) {
    result = commandActionHelper.handleReply(&message);
    if (result == returnvalue::OK) {
      continue;
    }
  }
}

ReturnValue_t DeviceHandlerCommander::sendCommand(object_id_t target, ActionId_t actionId) {
  return commandActionHelper.commandAction(target, actionId, nullptr, 0);
}

ReturnValue_t DeviceHandlerCommander::getReplyReturnCode() { return lastReplyReturnCode; }

void DeviceHandlerCommander::resetReplyReturnCode() { lastReplyReturnCode = returnvalue::FAILED; }

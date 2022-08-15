#include "DeviceHandlerMock.h"

#include <catch2/catch_test_macros.hpp>

DeviceHandlerMock::DeviceHandlerMock(object_id_t objectId, object_id_t deviceCommunication,
                                     CookieIF *comCookie, FailureIsolationBase *fdirInstance)
    : DeviceHandlerBase(objectId, deviceCommunication, comCookie, fdirInstance) {
  mode = MODE_ON;
}

DeviceHandlerMock::~DeviceHandlerMock() = default;

void DeviceHandlerMock::doStartUp() { setMode(_MODE_TO_ON); }

void DeviceHandlerMock::doShutDown() { setMode(_MODE_POWER_DOWN); }

ReturnValue_t DeviceHandlerMock::buildNormalDeviceCommand(DeviceCommandId_t *id) {
  return NOTHING_TO_SEND;
}

ReturnValue_t DeviceHandlerMock::buildTransitionDeviceCommand(DeviceCommandId_t *id) {
  return NOTHING_TO_SEND;
}

ReturnValue_t DeviceHandlerMock::buildCommandFromCommand(DeviceCommandId_t deviceCommand,
                                                         const uint8_t *commandData,
                                                         size_t commandDataLen) {
  switch (deviceCommand) {
    case SIMPLE_COMMAND: {
      commandBuffer[0] = SIMPLE_COMMAND_DATA;
      rawPacket = commandBuffer;
      rawPacketLen = sizeof(SIMPLE_COMMAND_DATA);
      break;
    }
    default:
      WARN("DeviceHandlerMock::buildCommandFromCommand: Invalid device command");
      break;
  }
  return returnvalue::OK;
}

ReturnValue_t DeviceHandlerMock::scanForReply(const uint8_t *start, size_t len,
                                              DeviceCommandId_t *foundId, size_t *foundLen) {
  switch (*start) {
    case SIMPLE_COMMAND_DATA: {
      *foundId = SIMPLE_COMMAND;
      *foundLen = sizeof(SIMPLE_COMMAND_DATA);
      return returnvalue::OK;
      break;
    }
    case PERIODIC_REPLY_DATA: {
      *foundId = PERIODIC_REPLY;
      *foundLen = sizeof(PERIODIC_REPLY_DATA);
      return returnvalue::OK;
      break;
    }
    default:
      break;
  }
  return returnvalue::FAILED;
}

ReturnValue_t DeviceHandlerMock::interpretDeviceReply(DeviceCommandId_t id, const uint8_t *packet) {
  switch (id) {
    case SIMPLE_COMMAND:
    case PERIODIC_REPLY: {
      periodicReplyReceived = true;
      break;
    }
    default:
      break;
  }
  return returnvalue::OK;
}

void DeviceHandlerMock::fillCommandAndReplyMap() {
  insertInCommandAndReplyMap(SIMPLE_COMMAND, 0, nullptr, 0, false, false, 0,
                             &simpleCommandReplyTimeout);
  insertInCommandAndReplyMap(PERIODIC_REPLY, 0, nullptr, 0, true, false, 0,
                             &periodicReplyCountdown);
}

uint32_t DeviceHandlerMock::getTransitionDelayMs(Mode_t modeFrom, Mode_t modeTo) { return 500; }

void DeviceHandlerMock::changePeriodicReplyCountdown(uint32_t timeout) {
  periodicReplyCountdown.setTimeout(timeout);
}

void DeviceHandlerMock::changeSimpleCommandReplyCountdown(uint32_t timeout) {
  simpleCommandReplyTimeout.setTimeout(timeout);
}

void DeviceHandlerMock::resetPeriodicReplyState() { periodicReplyReceived = false; }

bool DeviceHandlerMock::getPeriodicReplyReceived() { return periodicReplyReceived; }

ReturnValue_t DeviceHandlerMock::enablePeriodicReply(DeviceCommandId_t replyId) {
  return updatePeriodicReply(true, replyId);
}

ReturnValue_t DeviceHandlerMock::disablePeriodicReply(DeviceCommandId_t replyId) {
  return updatePeriodicReply(false, replyId);
}

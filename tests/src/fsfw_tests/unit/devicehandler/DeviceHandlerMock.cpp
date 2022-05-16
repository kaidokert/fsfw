#include "DeviceHandlerMock.h"

DeviceHandlerMock::DeviceHandlerMock(object_id_t objectId, object_id_t deviceCommunication)
    : DeviceHandlerBase(objetcId, deviceCommunication, nullptr) {}

DeviceHandlerMock::~DeviceHandlerMock() {
}

void DeviceHandlerMock::doStartup() {
  setMode(_MODE_TO_ON);
}

void DeviceHandlerMock::doShutdown() {
  setMode(_MODE_POWER_DOWN);
}

ReturnValue_t DeviceHandlerMock::buildNormalDeviceCommand(DeviceCommandId_t *id) {
  return NOTHING_TO_SEND;
}

ReturnValue_t DeviceHandlerMock::buildTransitionDeviceCommand(DeviceCommandId_t *id) {
  return NOTHING_TO_SEND;
}

ReturnValue_t DeviceHandlerMock::buildCommandFromCommand(DeviceCommandId_t deviceCommand,
                                                         const uint8_t *commandData,
                                                         size_t commandDataLen) {
  switch(deviceCommand) {
    case PERIODIC_REPLY_TEST_COMMAND: {
      commandBuffer[0] = periodicReplyTestData;
      rawPacket = commandBuffer;
      rawPacketLen = sizeof(periodicReplyTestData);
    }
    default:
      WARN("DeviceHandlerMock::buildCommandFromCommand: Invalid device command");
  }
  return RETURN_OK;
}

ReturnValue_t DeviceHandlerMock::scanForReply(const uint8_t *start, size_t len, DeviceCommandId_t *foundId,
                                     size_t *foundLen) {
  switch(*start) {
    case periodicReplyTestData: {
      return RETURN_OK;
      break;
    }
    default:
      break;
  }
  return RETURN_FAILED;
}

ReturnValue_t DeviceHandlerMock::interpretDeviceReply(DeviceCommandId_t id, const uint8_t *packet) {
  switch(id){
    case PERIODIC_REPLY_TEST_COMMAND:
      break;
    default:
      break;
  }
}

void DeviceHandlerMock::fillCommandAndReplyMap() {
  insertInCommandAndReplyMap(PERIODIC_REPLY_TEST_COMMAND, 2, nullptr, 0, true, false, 0,
                             periodicReplyCountdown);
}

uint32_t DeviceHandlerMock::getTransitionDelayMs(Mode_t modeFrom, Mode_t modeTo) {
  return 500;
}


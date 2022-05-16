#ifndef TESTS_SRC_FSFW_TESTS_UNIT_DEVICEHANDLER_DEVICEHANDLERMOCK_H_
#define TESTS_SRC_FSFW_TESTS_UNIT_DEVICEHANDLER_DEVICEHANDLERMOCK_H_

#include <fsfw/devicehandlers/DeviceHandlerBase.h>

class DeviceHandlerMock : public DeviceHandlerBase {
 public:

  static const DeviceCommandId_t PERIODIC_REPLY_TEST_COMMAND = 1;

  DeviceHandlerMock(object_id_t objectId, object_id_t deviceCommunication);
  virtual ~DeviceHandlerMock();

protected:
  void doStartUp() override;
  void doStartShutdown() override;
  ReturnValue_t buildNormalDeviceCommand(DeviceCommandId_t *id) override;
  ReturnValue_t buildTransitionDeviceCommand(DeviceCommandId_t *id) override;
  ReturnValue_t buildCommandFromCommand(DeviceCommandId_t deviceCommand, const uint8_t *commandData,
                                        size_t commandDataLen) override;
  ReturnValue_t scanForReply(const uint8_t *start, size_t len, DeviceCommandId_t *foundId,
                                       size_t *foundLen) override;
  ReturnValue_t interpretDeviceReply(DeviceCommandId_t id, const uint8_t *packet) override;
  void fillCommandAndReplyMap() override;
  uint32_t getTransitionDelayMs(Mode_t modeFrom, Mode_t modeTo) override;

private:

  uint8_t periodicReplyTestData = 1;

  Countdown periodicReplyCountdown = Countdown(10);

  uint8_t commandBuffer[1];
};

#endif /* TESTS_SRC_FSFW_TESTS_UNIT_DEVICEHANDLER_DEVICEHANDLERMOCK_H_ */

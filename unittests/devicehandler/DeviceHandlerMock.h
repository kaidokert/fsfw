#ifndef TESTS_SRC_FSFW_TESTS_UNIT_DEVICEHANDLER_DEVICEHANDLERMOCK_H_
#define TESTS_SRC_FSFW_TESTS_UNIT_DEVICEHANDLER_DEVICEHANDLERMOCK_H_

#include <fsfw/devicehandlers/DeviceHandlerBase.h>

class DeviceHandlerMock : public DeviceHandlerBase {
 public:
  static const DeviceCommandId_t SIMPLE_COMMAND = 1;
  static const DeviceCommandId_t PERIODIC_REPLY = 2;

  static const uint8_t SIMPLE_COMMAND_DATA = 1;
  static const uint8_t PERIODIC_REPLY_DATA = 2;

  DeviceHandlerMock(object_id_t objectId, object_id_t deviceCommunication, CookieIF *comCookie,
                    FailureIsolationBase *fdirInstance);
  virtual ~DeviceHandlerMock();
  void changePeriodicReplyCountdown(uint32_t timeout);
  void changeSimpleCommandReplyCountdown(uint32_t timeout);
  void resetPeriodicReplyState();
  bool getPeriodicReplyReceived();
  ReturnValue_t enablePeriodicReply(DeviceCommandId_t replyId);
  ReturnValue_t disablePeriodicReply(DeviceCommandId_t replyId);

 protected:
  void doStartUp() override;
  void doShutDown() override;
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
  Countdown simpleCommandReplyTimeout = Countdown(1000);
  Countdown periodicReplyCountdown = Countdown(1000);

  uint8_t commandBuffer[1];

  bool periodicReplyReceived = false;
};

#endif /* TESTS_SRC_FSFW_TESTS_UNIT_DEVICEHANDLER_DEVICEHANDLERMOCK_H_ */

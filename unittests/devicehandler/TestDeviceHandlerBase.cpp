#include <catch2/catch_test_macros.hpp>

#include "DeviceHandlerCommander.h"
#include "mocks/ComIFMock.h"
#include "mocks/CookieIFMock.h"
#include "mocks/DeviceFdirMock.h"
#include "mocks/DeviceHandlerMock.h"
#include "objects/systemObjectList.h"

TEST_CASE("Device Handler Base", "[DeviceHandlerBase]") {
  // Will be deleted with DHB destructor
  auto* cookieIFMock = new CookieIFMock;
  ComIFMock comIF(objects::COM_IF_MOCK);
  DeviceFdirMock deviceFdirMock(objects::DEVICE_HANDLER_MOCK, objects::NO_OBJECT);
  DeviceHandlerMock deviceHandlerMock(objects::DEVICE_HANDLER_MOCK, objects::COM_IF_MOCK,
                                      cookieIFMock, &deviceFdirMock);
  ReturnValue_t result = deviceHandlerMock.initialize();
  REQUIRE(result == returnvalue::OK);
  DeviceHandlerCommander deviceHandlerCommander(objects::DEVICE_HANDLER_COMMANDER);
  result = deviceHandlerCommander.initialize();
  REQUIRE(result == returnvalue::OK);

  SECTION("Commanding nominal") {
    comIF.setTestCase(ComIFMock::TestCase::SIMPLE_COMMAND_NOMINAL);
    result = deviceHandlerCommander.sendCommand(objects::DEVICE_HANDLER_MOCK,
                                                DeviceHandlerMock::SIMPLE_COMMAND);
    REQUIRE(result == returnvalue::OK);
    deviceHandlerMock.performOperation(DeviceHandlerIF::PERFORM_OPERATION);
    deviceHandlerMock.performOperation(DeviceHandlerIF::SEND_WRITE);
    deviceHandlerMock.performOperation(DeviceHandlerIF::GET_WRITE);
    deviceHandlerMock.performOperation(DeviceHandlerIF::SEND_READ);
    deviceHandlerMock.performOperation(DeviceHandlerIF::GET_READ);
    deviceHandlerCommander.performOperation();
    result = deviceHandlerCommander.getReplyReturnCode();
    uint32_t missedReplies = deviceFdirMock.getMissedReplyCount();
    REQUIRE(missedReplies == 0);
    REQUIRE(result == returnvalue::OK);
  }

  SECTION("Commanding missed reply") {
    comIF.setTestCase(ComIFMock::TestCase::MISSED_REPLY);
    deviceHandlerCommander.resetReplyReturnCode();
    // Set the timeout to 0 to immediately timeout the reply
    deviceHandlerMock.changeSimpleCommandReplyCountdown(0);
    result = deviceHandlerCommander.sendCommand(objects::DEVICE_HANDLER_MOCK,
                                                DeviceHandlerMock::SIMPLE_COMMAND);
    REQUIRE(result == returnvalue::OK);
    deviceHandlerMock.performOperation(DeviceHandlerIF::PERFORM_OPERATION);
    deviceHandlerMock.performOperation(DeviceHandlerIF::SEND_WRITE);
    deviceHandlerMock.performOperation(DeviceHandlerIF::GET_WRITE);
    deviceHandlerMock.performOperation(DeviceHandlerIF::SEND_READ);
    deviceHandlerMock.performOperation(DeviceHandlerIF::GET_READ);
    deviceHandlerMock.performOperation(DeviceHandlerIF::PERFORM_OPERATION);
    deviceHandlerCommander.performOperation();
    result = deviceHandlerCommander.getReplyReturnCode();
    REQUIRE(result == DeviceHandlerIF::TIMEOUT);
    uint32_t missedReplies = deviceFdirMock.getMissedReplyCount();
    REQUIRE(missedReplies == 1);
  }

  SECTION("Periodic reply nominal") {
    comIF.setTestCase(ComIFMock::TestCase::PERIODIC_REPLY_NOMINAL);
    deviceHandlerMock.enablePeriodicReply(DeviceHandlerMock::PERIODIC_REPLY);
    deviceHandlerMock.performOperation(DeviceHandlerIF::PERFORM_OPERATION);
    deviceHandlerMock.performOperation(DeviceHandlerIF::SEND_WRITE);
    deviceHandlerMock.performOperation(DeviceHandlerIF::GET_WRITE);
    deviceHandlerMock.performOperation(DeviceHandlerIF::SEND_READ);
    deviceHandlerMock.performOperation(DeviceHandlerIF::GET_READ);
    REQUIRE(deviceHandlerMock.getPeriodicReplyReceived() == true);
  }

  SECTION("Missed periodic reply") {
    comIF.setTestCase(ComIFMock::TestCase::MISSED_REPLY);
    // Set the timeout to 0 to immediately timeout the reply
    deviceHandlerMock.changePeriodicReplyCountdown(0);
    deviceHandlerMock.enablePeriodicReply(DeviceHandlerMock::PERIODIC_REPLY);
    deviceHandlerMock.performOperation(DeviceHandlerIF::PERFORM_OPERATION);
    deviceHandlerMock.performOperation(DeviceHandlerIF::SEND_WRITE);
    deviceHandlerMock.performOperation(DeviceHandlerIF::GET_WRITE);
    deviceHandlerMock.performOperation(DeviceHandlerIF::SEND_READ);
    deviceHandlerMock.performOperation(DeviceHandlerIF::GET_READ);
    uint32_t missedReplies = deviceFdirMock.getMissedReplyCount();
    REQUIRE(missedReplies == 1);
    // Test if disabling of periodic reply
    deviceHandlerMock.disablePeriodicReply(DeviceHandlerMock::PERIODIC_REPLY);
    deviceHandlerMock.performOperation(DeviceHandlerIF::PERFORM_OPERATION);
    deviceHandlerMock.performOperation(DeviceHandlerIF::SEND_WRITE);
    deviceHandlerMock.performOperation(DeviceHandlerIF::GET_WRITE);
    deviceHandlerMock.performOperation(DeviceHandlerIF::SEND_READ);
    deviceHandlerMock.performOperation(DeviceHandlerIF::GET_READ);
    missedReplies = deviceFdirMock.getMissedReplyCount();
    // Should still be 1 because periodic reply is now disabled
    REQUIRE(missedReplies == 1);
  }
}

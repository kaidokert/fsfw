#include <catch2/catch_test_macros.hpp>

#include "ComIFMock.h"
#include "DeviceFdirMock.h"
#include "fsfw_tests/unit/devicehandler/CookieIFMock.h"
#include "fsfw_tests/unit/devicehandler/DeviceHandlerCommander.h"
#include "fsfw_tests/unit/devicehandler/DeviceHandlerMock.h"
#include "fsfw_tests/unit/testcfg/objects/systemObjectList.h"

TEST_CASE("Device Handler Base", "[DeviceHandlerBase]") {

	// Will be deleted with DHB destructor
	CookieIFMock* cookieIFMock = new CookieIFMock;
	ComIFMock comIF(objects::COM_IF_MOCK);
	DeviceFdirMock deviceFdirMock(objects::DEVICE_HANDLER_MOCK, objects::NO_OBJECT);
	DeviceHandlerMock deviceHandlerMock(objects::DEVICE_HANDLER_MOCK, objects::COM_IF_MOCK,
																			cookieIFMock, &deviceFdirMock);
	ReturnValue_t result = deviceHandlerMock.initialize();
	REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
	DeviceHandlerCommander deviceHandlerCommander(objects::DEVICE_HANDLER_COMMANDER);
	result = deviceHandlerCommander.initialize();
	REQUIRE(result == HasReturnvaluesIF::RETURN_OK);

  SECTION("Commanding") {
  	comIF.setTestCase(ComIFMock::TestCase::SIMPLE_COMMAND_NOMINAL);
    result = deviceHandlerCommander.sendCommand(objects::DEVICE_HANDLER_MOCK,
                                                DeviceHandlerMock::SIMPLE_COMMAND);
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
    deviceHandlerMock.performOperation(DeviceHandlerIF::PERFORM_OPERATION);
    deviceHandlerMock.performOperation(DeviceHandlerIF::SEND_WRITE);
    deviceHandlerMock.performOperation(DeviceHandlerIF::GET_WRITE);
    deviceHandlerMock.performOperation(DeviceHandlerIF::SEND_READ);
    deviceHandlerMock.performOperation(DeviceHandlerIF::GET_READ);
    deviceHandlerCommander.performOperation();
    result = deviceHandlerCommander.getReplyReturnCode();
    REQUIRE(result == HasReturnvaluesIF::RETURN_OK);
  }

  SECTION("Periodic reply") {
  	comIF.setTestCase(ComIFMock::TestCase::PERIODIC_REPLY_NOMINAL);
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
    deviceHandlerMock.performOperation(DeviceHandlerIF::PERFORM_OPERATION);
    deviceHandlerMock.performOperation(DeviceHandlerIF::SEND_WRITE);
    deviceHandlerMock.performOperation(DeviceHandlerIF::GET_WRITE);
    deviceHandlerMock.performOperation(DeviceHandlerIF::SEND_READ);
    deviceHandlerMock.performOperation(DeviceHandlerIF::GET_READ);
    uint32_t missedReplies = deviceFdirMock.getMissedReplyCount();
    REQUIRE(missedReplies == 1);
  }
}

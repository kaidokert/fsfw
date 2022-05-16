#include "TestDeviceHandlerBase.h"

#include <catch2/catch_test_macros.hpp>
#include "ComIFMock.h"

#include "fsfw_tests/unit/devicehandler/DeviceHandlerMock.h"
#include "fsfw_tests/unit/devicehandler/DeviceHandlerCommander.h"
#include "fsfw_tests/unit/devicehandler/CookieIFMock.h"
#include "fsfw_tests/unit/testcfg/objects/systemObjectList.h"

TEST_CASE("Device Handler Base", "[DeviceHandlerBase]") {

  SECTION("Periodic reply with countdown based timeout success") {
    CookieIFMock cookieIFMock;
    ComIFperiodicMock comIFperiodic(objects::COM_IF_MOCK);
    DeviceHandlerMock deviceHandlerMock(objects::DEVICE_HANDLER_MOCK, objects::COM_IF_MOCK,
                                        &cookieIFMock);
    ReturnValue_t result = deviceHandlerMock.initialize();
    REQUIRE(result == RETURN_OK);
    DeviceHandlerCommander deviceHandlerCommander(objects::DEVICE_HANDLER_COMMANDER);
    result = deviceHandlerCommander.initialize();
    REQUIRE(result == RETURN_OK);
    result = deviceHandlerCommander.sendCommand(objects::DEVICE_HANDLER_MOCK,
                                                DeviceHandlerMock::PERIODIC_REPLY_TEST_COMMAND);
    deviceHandlerMock.performOperation(DeviceHandlerIF::PERFORM_OPERATION);
    deviceHandlerMock.performOperation(DeviceHandlerIF::SEND_WRITE);
    deviceHandlerMock.performOperation(DeviceHandlerIF::GET_WRITE);
    deviceHandlerMock.performOperation(DeviceHandlerIF::SEND_READ);
    deviceHandlerMock.performOperation(DeviceHandlerIF::GET_READ);
    result = deviceHandlerCommander.getReplyReturnCode();
    REQUIRE(result == RETURN_OK);
  }
//  SECTION("Periodic reply with countdown based timeout failed") {
//
//  }
}

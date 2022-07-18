#include "TestActionHelper.h"

#include <fsfw/action/ActionHelper.h>
#include <fsfw/ipc/CommandMessage.h>

#include <array>
#include <catch2/catch_test_macros.hpp>

#include "mocks/MessageQueueMockBase.h"

TEST_CASE("Action Helper", "[ActionHelper]") {
  ActionHelperOwnerMockBase testDhMock;
  MessageQueueMockBase testMqMock;
  ActionHelper actionHelper = ActionHelper(&testDhMock, dynamic_cast<MessageQueueIF*>(&testMqMock));
  CommandMessage actionMessage;
  ActionId_t testActionId = 777;
  std::array<uint8_t, 3> testParams{1, 2, 3};
  store_address_t paramAddress;
  StorageManagerIF* ipcStore = tglob::getIpcStoreHandle();
  REQUIRE(ipcStore != nullptr);
  ipcStore->addData(&paramAddress, testParams.data(), 3);
  REQUIRE(actionHelper.initialize() == retval::CATCH_OK);

  SECTION("Simple tests") {
    ActionMessage::setCommand(&actionMessage, testActionId, paramAddress);
    CHECK(not testDhMock.executeActionCalled);
    REQUIRE(actionHelper.handleActionMessage(&actionMessage) == retval::CATCH_OK);
    CHECK(testDhMock.executeActionCalled);
    // No message is sent if everything is alright.
    CHECK(not testMqMock.wasMessageSent());
    store_address_t invalidAddress;
    ActionMessage::setCommand(&actionMessage, testActionId, invalidAddress);
    actionHelper.handleActionMessage(&actionMessage);
    CHECK(testMqMock.wasMessageSent());
    const uint8_t* ptr = nullptr;
    size_t size = 0;
    REQUIRE(ipcStore->getData(paramAddress, &ptr, &size) ==
            static_cast<uint32_t>(StorageManagerIF::DATA_DOES_NOT_EXIST));
    REQUIRE(ptr == nullptr);
    REQUIRE(size == 0);
    testDhMock.getBuffer(&ptr, &size);
    REQUIRE(size == 3);
    for (uint8_t i = 0; i < 3; i++) {
      REQUIRE(ptr[i] == (i + 1));
    }
    testDhMock.clearBuffer();
  }

  SECTION("Handle failures") {
    actionMessage.setCommand(1234);
    REQUIRE(actionHelper.handleActionMessage(&actionMessage) ==
            static_cast<uint32_t>(CommandMessage::UNKNOWN_COMMAND));
    CHECK(not testMqMock.wasMessageSent());
    uint16_t step = 5;
    ReturnValue_t status = 0x1234;
    actionHelper.step(step, testMqMock.getId(), testActionId, status);
    step += 1;
    CHECK(testMqMock.wasMessageSent());
    CommandMessage testMessage;
    REQUIRE(testMqMock.receiveMessage(&testMessage) ==
            static_cast<uint32_t>(HasReturnvaluesIF::RETURN_OK));
    REQUIRE(testMessage.getCommand() == static_cast<uint32_t>(ActionMessage::STEP_FAILED));
    REQUIRE(testMessage.getParameter() == static_cast<uint32_t>(testActionId));
    uint32_t parameter2 = ((uint32_t)step << 16) | (uint32_t)status;
    REQUIRE(testMessage.getParameter2() == parameter2);
    REQUIRE(ActionMessage::getStep(&testMessage) == step);
  }

  SECTION("Handle finish") {
    CHECK(not testMqMock.wasMessageSent());
    ReturnValue_t status = 0x9876;
    actionHelper.finish(false, testMqMock.getId(), testActionId, status);
    CHECK(testMqMock.wasMessageSent());
    CommandMessage testMessage;
    REQUIRE(testMqMock.receiveMessage(&testMessage) ==
            static_cast<uint32_t>(HasReturnvaluesIF::RETURN_OK));
    REQUIRE(testMessage.getCommand() == static_cast<uint32_t>(ActionMessage::COMPLETION_FAILED));
    REQUIRE(ActionMessage::getActionId(&testMessage) == testActionId);
    REQUIRE(ActionMessage::getReturnCode(&testMessage) == static_cast<uint32_t>(status));
  }

  SECTION("Handle failed") {
    store_address_t toLongParamAddress = StorageManagerIF::INVALID_ADDRESS;
    std::array<uint8_t, 5> toLongData = {5, 4, 3, 2, 1};
    REQUIRE(ipcStore->addData(&toLongParamAddress, toLongData.data(), 5) == retval::CATCH_OK);
    ActionMessage::setCommand(&actionMessage, testActionId, toLongParamAddress);
    CHECK(not testDhMock.executeActionCalled);
    REQUIRE(actionHelper.handleActionMessage(&actionMessage) == retval::CATCH_OK);
    REQUIRE(ipcStore->getData(toLongParamAddress).first ==
            static_cast<uint32_t>(StorageManagerIF::DATA_DOES_NOT_EXIST));
    CommandMessage testMessage;
    REQUIRE(testMqMock.receiveMessage(&testMessage) ==
            static_cast<uint32_t>(HasReturnvaluesIF::RETURN_OK));
    REQUIRE(testMessage.getCommand() == static_cast<uint32_t>(ActionMessage::STEP_FAILED));
    REQUIRE(ActionMessage::getReturnCode(&testMessage) == 0xAFFE);
    REQUIRE(ActionMessage::getStep(&testMessage) == 0);
    REQUIRE(ActionMessage::getActionId(&testMessage) == testActionId);
  }

  SECTION("Missing IPC Data") {
    ActionMessage::setCommand(&actionMessage, testActionId, StorageManagerIF::INVALID_ADDRESS);
    CHECK(not testDhMock.executeActionCalled);
    REQUIRE(actionHelper.handleActionMessage(&actionMessage) == retval::CATCH_OK);
    CommandMessage testMessage;
    REQUIRE(testMqMock.receiveMessage(&testMessage) ==
            static_cast<uint32_t>(HasReturnvaluesIF::RETURN_OK));
    REQUIRE(testMessage.getCommand() == static_cast<uint32_t>(ActionMessage::STEP_FAILED));
    REQUIRE(ActionMessage::getReturnCode(&testMessage) ==
            static_cast<uint32_t>(StorageManagerIF::ILLEGAL_STORAGE_ID));
    REQUIRE(ActionMessage::getStep(&testMessage) == 0);
  }

  SECTION("Data Reply") {}
}

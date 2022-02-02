#include "fsfw/datapool/HkSwitchHelper.h"

#include "fsfw/ipc/QueueFactory.h"

HkSwitchHelper::HkSwitchHelper(EventReportingProxyIF* eventProxy)
    : commandActionHelper(this), eventProxy(eventProxy) {
  actionQueue = QueueFactory::instance()->createMessageQueue();
}

HkSwitchHelper::~HkSwitchHelper() { QueueFactory::instance()->deleteMessageQueue(actionQueue); }

ReturnValue_t HkSwitchHelper::initialize() {
  ReturnValue_t result = commandActionHelper.initialize();

  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }

  return result;
}

ReturnValue_t HkSwitchHelper::performOperation(uint8_t operationCode) {
  CommandMessage command;
  while (actionQueue->receiveMessage(&command) == HasReturnvaluesIF::RETURN_OK) {
    ReturnValue_t result = commandActionHelper.handleReply(&command);
    if (result == HasReturnvaluesIF::RETURN_OK) {
      continue;
    }
    command.setToUnknownCommand();
    actionQueue->reply(&command);
  }

  return HasReturnvaluesIF::RETURN_OK;
}

void HkSwitchHelper::stepSuccessfulReceived(ActionId_t actionId, uint8_t step) {}

void HkSwitchHelper::stepFailedReceived(ActionId_t actionId, uint8_t step,
                                        ReturnValue_t returnCode) {
  eventProxy->forwardEvent(SWITCHING_TM_FAILED, returnCode, actionId);
}

void HkSwitchHelper::dataReceived(ActionId_t actionId, const uint8_t* data, uint32_t size) {}

void HkSwitchHelper::completionSuccessfulReceived(ActionId_t actionId) {}

void HkSwitchHelper::completionFailedReceived(ActionId_t actionId, ReturnValue_t returnCode) {
  eventProxy->forwardEvent(SWITCHING_TM_FAILED, returnCode, actionId);
}

ReturnValue_t HkSwitchHelper::switchHK(SerializeIF* sids, bool enable) {
  //	ActionId_t action = HKService::DISABLE_HK;
  //	if (enable) {
  //		action = HKService::ENABLE_HK;
  //	}
  //
  //	ReturnValue_t result = commandActionHelper.commandAction(
  //			objects::PUS_HK_SERVICE, action, sids);
  //
  //	if (result != HasReturnvaluesIF::RETURN_OK) {
  //		eventProxy->forwardEvent(SWITCHING_TM_FAILED, result);
  //	}
  //	return result;
  return HasReturnvaluesIF::RETURN_OK;
}

MessageQueueIF* HkSwitchHelper::getCommandQueuePtr() { return actionQueue; }

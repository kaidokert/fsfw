#include "HkSwitchHelper.h"
//#include <mission/tmtcservices/HKService_03.h>
#include "../ipc/QueueFactory.h"

HkSwitchHelper::HkSwitchHelper(EventReportingProxyIF* eventProxy) :
		commandActionHelper(this), eventProxy(eventProxy) {
	actionQueue = QueueFactory::instance()->createMessageQueue();
}

HkSwitchHelper::~HkSwitchHelper() {
	// TODO Auto-generated destructor stub
}

ReturnValue_t HkSwitchHelper::initialize() {
	ReturnValue_t result = commandActionHelper.initialize();

	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	return result;
}

ReturnValue_t HkSwitchHelper::performOperation(uint8_t operationCode) {
	CommandMessage message;
	while (actionQueue->receiveMessage(&message) == HasReturnvaluesIF::RETURN_OK) {
		ReturnValue_t result = commandActionHelper.handleReply(&message);
		if (result == HasReturnvaluesIF::RETURN_OK) {
			continue;
		}
		message.setToUnknownCommand();
		actionQueue->reply(&message);
	}

	return HasReturnvaluesIF::RETURN_OK;
}

void HkSwitchHelper::stepSuccessfulReceived(ActionId_t actionId, uint8_t step) {
}

void HkSwitchHelper::stepFailedReceived(ActionId_t actionId, uint8_t step,
		ReturnValue_t returnCode) {
	eventProxy->forwardEvent(SWITCHING_TM_FAILED, returnCode, actionId);
}

void HkSwitchHelper::dataReceived(ActionId_t actionId, const uint8_t* data,
		uint32_t size) {
}

void HkSwitchHelper::completionSuccessfulReceived(ActionId_t actionId) {
}

void HkSwitchHelper::completionFailedReceived(ActionId_t actionId,
		ReturnValue_t returnCode) {
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

MessageQueueIF* HkSwitchHelper::getCommandQueuePtr() {
	return actionQueue;
}

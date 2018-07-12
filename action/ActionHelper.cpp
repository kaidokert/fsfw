#include <framework/action/ActionHelper.h>
#include <framework/action/HasActionsIF.h>
#include <framework/objectmanager/ObjectManagerIF.h>
ActionHelper::ActionHelper(HasActionsIF* setOwner, MessageQueueIF* useThisQueue) :
		owner(setOwner), queueToUse(useThisQueue), ipcStore(
				NULL) {
}

ActionHelper::~ActionHelper() {
}

ReturnValue_t ActionHelper::handleActionMessage(CommandMessage* command) {
	if (command->getCommand() == ActionMessage::EXECUTE_ACTION) {
		ActionId_t currentAction = ActionMessage::getActionId(command);
		prepareExecution(command->getSender(), currentAction,
				ActionMessage::getStoreId(command));
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return CommandMessage::UNKNOW_COMMAND;
	}
}

ReturnValue_t ActionHelper::initialize() {
	ipcStore = objectManager->get<StorageManagerIF>(objects::IPC_STORE);
	if (ipcStore == NULL) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

void ActionHelper::step(uint8_t step, MessageQueueId_t reportTo, ActionId_t commandId, ReturnValue_t result) {
	CommandMessage reply;
	ActionMessage::setStepReply(&reply, commandId, step + STEP_OFFSET, result);
	queueToUse->sendMessage(reportTo, &reply);
}

void ActionHelper::finish(MessageQueueId_t reportTo, ActionId_t commandId, ReturnValue_t result) {
	CommandMessage reply;
	ActionMessage::setCompletionReply(&reply, commandId, result);
	queueToUse->sendMessage(reportTo, &reply);
}

void ActionHelper::setQueueToUse(MessageQueueIF* queue) {
	queueToUse = queue;
}

void ActionHelper::prepareExecution(MessageQueueId_t commandedBy, ActionId_t actionId,
		store_address_t dataAddress) {
	const uint8_t* dataPtr = NULL;
	uint32_t size = 0;
	ReturnValue_t result = ipcStore->getData(dataAddress, &dataPtr, &size);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		CommandMessage reply;
		ActionMessage::setStepReply(&reply, actionId, 0, result);
		queueToUse->sendMessage(commandedBy, &reply);
		return;
	}
	result = owner->executeAction(actionId, commandedBy, dataPtr, size);
	ipcStore->deleteData(dataAddress);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		CommandMessage reply;
		ActionMessage::setStepReply(&reply, actionId, 0, result);
		queueToUse->sendMessage(commandedBy, &reply);
		return;
	}
}

ReturnValue_t ActionHelper::reportData(MessageQueueId_t reportTo, ActionId_t replyId, SerializeIF* data, bool hideSender) {
	CommandMessage reply;
	store_address_t storeAddress;
	uint8_t *dataPtr;
	uint32_t maxSize = data->getSerializedSize();
	if (maxSize == 0) {
		//No error, there's simply nothing to report.
		return HasReturnvaluesIF::RETURN_OK;
	}
	uint32_t size = 0;
	ReturnValue_t result = ipcStore->getFreeElement(&storeAddress, maxSize,
			&dataPtr);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	result = data->serialize(&dataPtr, &size, maxSize, true);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		ipcStore->deleteData(storeAddress);
		return result;
	}
	//We don't need to report the objectId, as we receive REQUESTED data before the completion success message.
	//True aperiodic replies need to be reported with another dedicated message.
	ActionMessage::setDataReply(&reply, replyId, storeAddress);

	//TODO Service Implementation sucks at the moment
	if (hideSender){
		result = MessageQueueSenderIF::sendMessage(reportTo, &reply);
	} else {
		result = queueToUse->sendMessage(reportTo, &reply);
	}
	if ( result != HasReturnvaluesIF::RETURN_OK){
		ipcStore->deleteData(storeAddress);
	}
	return result;
}

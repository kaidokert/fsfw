#include "Service8FunctionManagement.h"
#include "servicepackets/Service8Packets.h"

#include "../objectmanager/SystemObjectIF.h"
#include "../action/HasActionsIF.h"
#include "../devicehandlers/DeviceHandlerIF.h"
#include "../serialize/SerializeAdapter.h"
#include "../serviceinterface/ServiceInterfaceStream.h"

Service8FunctionManagement::Service8FunctionManagement(object_id_t object_id,
		uint16_t apid, uint8_t serviceId, uint8_t numParallelCommands,
		uint16_t commandTimeoutSeconds):
		CommandingServiceBase(object_id, apid, serviceId, numParallelCommands,
		commandTimeoutSeconds) {}

Service8FunctionManagement::~Service8FunctionManagement() {}


ReturnValue_t Service8FunctionManagement::isValidSubservice(
		uint8_t subservice) {
	switch(static_cast<Subservice>(subservice)) {
	case Subservice::DIRECT_COMMANDING:
		return HasReturnvaluesIF::RETURN_OK;
	default:
		return AcceptsTelecommandsIF::INVALID_SUBSERVICE;
	}
}

ReturnValue_t Service8FunctionManagement::getMessageQueueAndObject(
		uint8_t subservice, const uint8_t* tcData, size_t tcDataLen,
		MessageQueueId_t* id, object_id_t* objectId) {
	if(tcDataLen < sizeof(object_id_t)) {
		return CommandingServiceBase::INVALID_TC;
	}
	SerializeAdapter::deSerialize(objectId, &tcData,
			&tcDataLen, SerializeIF::Endianness::BIG);

	return checkInterfaceAndAcquireMessageQueue(id,objectId);
}

ReturnValue_t Service8FunctionManagement::checkInterfaceAndAcquireMessageQueue(
		MessageQueueId_t* messageQueueToSet, object_id_t* objectId) {
	// check HasActionIF property of target
	HasActionsIF* possibleTarget = objectManager->get<HasActionsIF>(*objectId);
	if(possibleTarget == nullptr){
		return CommandingServiceBase::INVALID_OBJECT;
	}
	*messageQueueToSet = possibleTarget->getCommandQueue();
	return HasReturnvaluesIF::RETURN_OK;
}


ReturnValue_t Service8FunctionManagement::prepareCommand(
		CommandMessage* message, uint8_t subservice, const uint8_t* tcData,
		size_t tcDataLen, uint32_t* state, object_id_t objectId) {
	return prepareDirectCommand(dynamic_cast<CommandMessage*>(message),
				tcData, tcDataLen);
}

ReturnValue_t Service8FunctionManagement::prepareDirectCommand(
		CommandMessage *message, const uint8_t *tcData, size_t tcDataLen) {
    if(tcDataLen < sizeof(object_id_t) + sizeof(ActionId_t)) {
        sif::debug << "Service8FunctionManagement::prepareDirectCommand:"
                << " TC size smaller thant minimum size of direct command."
                << std::endl;
        return CommandingServiceBase::INVALID_TC;
    }

	// Create direct command instance by extracting data from Telecommand
	DirectCommand command(tcData, tcDataLen);

	// store additional parameters into the IPC Store
	store_address_t parameterAddress;
	ReturnValue_t result = IPCStore->addData(&parameterAddress,
			command.getParameters(),command.getParametersSize());

	// setCommand expects a Command Message, an Action ID and a store adress
	// pointing to additional parameters
	ActionMessage::setCommand(message,command.getActionId(),parameterAddress);
	return result;
}


ReturnValue_t Service8FunctionManagement::handleReply(
		const CommandMessage* reply, Command_t previousCommand,
		uint32_t* state, CommandMessage* optionalNextCommand,
		object_id_t objectId, bool* isStep) {
	Command_t replyId = reply->getCommand();
	ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
	ActionId_t actionId = ActionMessage::getActionId(reply);
	ReturnValue_t returnCode = ActionMessage::getReturnCode(reply);

	switch(replyId)  {
	case ActionMessage::COMPLETION_SUCCESS: {
		DirectReply completionReply(objectId, actionId,returnCode);
		result = CommandingServiceBase::EXECUTION_COMPLETE;
		break;
	}
	case ActionMessage::STEP_SUCCESS: {
		*isStep = true;
		result = HasReturnvaluesIF::RETURN_OK;
		break;
	}
	case ActionMessage::DATA_REPLY: {
	    result = handleDataReply(reply, objectId, actionId);
		break;
	}
	case ActionMessage::STEP_FAILED:
		*isStep = true;
		/*No break, falls through*/
	case ActionMessage::COMPLETION_FAILED:
		result = ActionMessage::getReturnCode(reply);
		break;
	default:
		result = INVALID_REPLY;
	}
	return result;
}

ReturnValue_t Service8FunctionManagement::handleDataReply(
        const CommandMessage* reply, object_id_t objectId,
        ActionId_t actionId) {
    store_address_t storeId = ActionMessage::getStoreId(reply);
    size_t size = 0;
    const uint8_t * buffer = nullptr;
    ReturnValue_t result = IPCStore->getData(storeId, &buffer, &size);
    if(result != RETURN_OK) {
        sif::error << "Service 8: Could not retrieve data for data reply"
                << std::endl;
        return result;
    }
    DataReply dataReply(objectId, actionId, buffer, size);
    result = sendTmPacket(static_cast<uint8_t>(
            Subservice::DIRECT_COMMANDING_DATA_REPLY), &dataReply);

    auto deletionResult = IPCStore->deleteData(storeId);
    if(deletionResult != HasReturnvaluesIF::RETURN_OK) {
        sif::warning << "Service8FunctionManagement::handleReply: Deletion"
                << " of data in pool failed." << std::endl;
    }
    return result;
}

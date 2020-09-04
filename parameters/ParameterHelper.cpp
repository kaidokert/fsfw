#include "ParameterHelper.h"
#include "ParameterMessage.h"
#include "../objectmanager/ObjectManagerIF.h"

ParameterHelper::ParameterHelper(ReceivesParameterMessagesIF* owner) :
		owner(owner) {}

ParameterHelper::~ParameterHelper() {
}

ReturnValue_t ParameterHelper::handleParameterMessage(CommandMessage *message) {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
	switch (message->getCommand()) {
	case ParameterMessage::CMD_PARAMETER_DUMP: {
		ParameterWrapper description;
		uint8_t domain = HasParametersIF::getDomain(
				ParameterMessage::getParameterId(message));
		uint16_t parameterId = HasParametersIF::getMatrixId(
				ParameterMessage::getParameterId(message));
		result = owner->getParameter(domain, parameterId,
				&description, &description, 0);
		if (result == HasReturnvaluesIF::RETURN_OK) {
			result = sendParameter(message->getSender(),
					ParameterMessage::getParameterId(message), &description);
		}
	}
		break;
	case ParameterMessage::CMD_PARAMETER_LOAD: {
		uint8_t domain = HasParametersIF::getDomain(
				ParameterMessage::getParameterId(message));
		uint16_t parameterId = HasParametersIF::getMatrixId(
				ParameterMessage::getParameterId(message));
		uint8_t index = HasParametersIF::getIndex(
				ParameterMessage::getParameterId(message));

		const uint8_t *storedStream = nullptr;
		size_t storedStreamSize = 0;
		result = storage->getData(
				ParameterMessage::getStoreId(message), &storedStream,
				&storedStreamSize);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			sif::error << "ParameterHelper::handleParameterMessage: Getting"
					" store data failed for load command." << std::endl;
			break;
		}

		ParameterWrapper streamWrapper;
		result = streamWrapper.set(storedStream, storedStreamSize);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			storage->deleteData(ParameterMessage::getStoreId(message));
			break;
		}

		ParameterWrapper ownerWrapper;
		result = owner->getParameter(domain, parameterId, &ownerWrapper,
				&streamWrapper, index);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			storage->deleteData(ParameterMessage::getStoreId(message));
			break;
		}

		result = ownerWrapper.copyFrom(&streamWrapper, index);

		storage->deleteData(ParameterMessage::getStoreId(message));

		if (result == HasReturnvaluesIF::RETURN_OK) {
			result = sendParameter(message->getSender(),
					ParameterMessage::getParameterId(message), &ownerWrapper);
		}
	}
		break;
	default:
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	if (result != HasReturnvaluesIF::RETURN_OK) {
		rejectCommand(message->getSender(), result, message->getCommand());
	}

	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t ParameterHelper::sendParameter(MessageQueueId_t to, uint32_t id,
		const ParameterWrapper* description) {
	size_t serializedSize = description->getSerializedSize();

	uint8_t *storeElement;
	store_address_t address;

	ReturnValue_t result = storage->getFreeElement(&address, serializedSize,
			&storeElement);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	size_t storeElementSize = 0;

	result = description->serialize(&storeElement, &storeElementSize,
			serializedSize, SerializeIF::Endianness::BIG);

	if (result != HasReturnvaluesIF::RETURN_OK) {
		storage->deleteData(address);
		return result;
	}

	CommandMessage reply;

	ParameterMessage::setParameterDumpReply(&reply, id, address);

	MessageQueueSenderIF::sendMessage(to, &reply, ownerQueueId);

	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t ParameterHelper::initialize() {
	ownerQueueId = owner->getCommandQueue();


	storage = objectManager->get<StorageManagerIF>(objects::IPC_STORE);
	if (storage == NULL) {
		return HasReturnvaluesIF::RETURN_FAILED;
	} else {
		return HasReturnvaluesIF::RETURN_OK;
	}
}

void ParameterHelper::rejectCommand(MessageQueueId_t to, ReturnValue_t reason,
		Command_t initialCommand) {
	CommandMessage reply;
	reply.setReplyRejected(reason, initialCommand);
	MessageQueueSenderIF::sendMessage(to, &reply, ownerQueueId);
}

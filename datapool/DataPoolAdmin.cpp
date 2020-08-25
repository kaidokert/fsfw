#include "DataPool.h"
#include "DataPoolAdmin.h"
#include "DataSet.h"
#include "PoolRawAccess.h"
#include "../ipc/CommandMessage.h"
#include "../ipc/QueueFactory.h"
#include "../parameters/ParameterMessage.h"

DataPoolAdmin::DataPoolAdmin(object_id_t objectId) :
		SystemObject(objectId), storage(NULL), commandQueue(NULL), memoryHelper(
				this, NULL), actionHelper(this, NULL) {
	commandQueue = QueueFactory::instance()->createMessageQueue();
}

DataPoolAdmin::~DataPoolAdmin() {
	QueueFactory::instance()->deleteMessageQueue(commandQueue);
}

ReturnValue_t DataPoolAdmin::performOperation(uint8_t opCode) {
	handleCommand();
	return RETURN_OK;
}

MessageQueueId_t DataPoolAdmin::getCommandQueue() const {
	return commandQueue->getId();
}

ReturnValue_t DataPoolAdmin::executeAction(ActionId_t actionId,
		MessageQueueId_t commandedBy, const uint8_t* data, size_t size) {
	if (actionId != SET_VALIDITY) {
		return INVALID_ACTION_ID;
	}

	if (size != 5) {
		return INVALID_PARAMETERS;
	}

	uint32_t address = (data[0] << 24) | (data[1] << 16) | (data[2] << 8)
			| data[3];

	uint8_t valid = data[4];

	uint32_t poolId = ::dataPool.PIDToDataPoolId(address);

	DataSet mySet;
	PoolRawAccess variable(poolId, 0, &mySet, PoolVariableIF::VAR_READ_WRITE);
	ReturnValue_t status = mySet.read();
	if (status != RETURN_OK) {
		return INVALID_ADDRESS;
	}
	if (valid != 0) {
		variable.setValid(PoolVariableIF::VALID);
	} else {
		variable.setValid(PoolVariableIF::INVALID);
	}

	mySet.commit();

	return EXECUTION_FINISHED;
}

ReturnValue_t DataPoolAdmin::getParameter(uint8_t domainId,
		uint16_t parameterId, ParameterWrapper* parameterWrapper,
		const ParameterWrapper* newValues, uint16_t startAtIndex) {
	return HasReturnvaluesIF::RETURN_FAILED;
}

void DataPoolAdmin::handleCommand() {
	CommandMessage command;
	ReturnValue_t result = commandQueue->receiveMessage(&command);
	if (result != RETURN_OK) {
		return;
	}

	result = actionHelper.handleActionMessage(&command);

	if (result == HasReturnvaluesIF::RETURN_OK) {
		return;
	}

	result = handleParameterCommand(&command);
	if (result == HasReturnvaluesIF::RETURN_OK) {
		return;
	}

	result = memoryHelper.handleMemoryCommand(&command);
	if (result != RETURN_OK) {
		command.setToUnknownCommand();
		commandQueue->reply(&command);
	}
}

ReturnValue_t DataPoolAdmin::handleMemoryLoad(uint32_t address,
		const uint8_t* data, size_t size, uint8_t** dataPointer) {
	uint32_t poolId = ::dataPool.PIDToDataPoolId(address);
	uint8_t arrayIndex = ::dataPool.PIDToArrayIndex(address);
	DataSet testSet;
	PoolRawAccess varToGetSize(poolId, arrayIndex, &testSet,
			PoolVariableIF::VAR_READ);
	ReturnValue_t status = testSet.read();
	if (status != RETURN_OK) {
		return INVALID_ADDRESS;
	}
	uint8_t typeSize = varToGetSize.getSizeOfType();

	if (size % typeSize != 0) {
		return INVALID_SIZE;
	}

	if (size > varToGetSize.getSizeTillEnd()) {
		return INVALID_SIZE;
	}
	const uint8_t* readPosition = data;

	for (; size > 0; size -= typeSize) {
		DataSet rawSet;
		PoolRawAccess variable(poolId, arrayIndex, &rawSet,
				PoolVariableIF::VAR_READ_WRITE);
		status = rawSet.read();
		if (status == RETURN_OK) {
			status = variable.setEntryFromBigEndian(readPosition, typeSize);
			if (status == RETURN_OK) {
				status = rawSet.commit();
			}
		}
		arrayIndex += 1;
		readPosition += typeSize;
	}
	return ACTIVITY_COMPLETED;
}

ReturnValue_t DataPoolAdmin::handleMemoryDump(uint32_t address, size_t size,
		uint8_t** dataPointer, uint8_t* copyHere) {
	uint32_t poolId = ::dataPool.PIDToDataPoolId(address);
	uint8_t arrayIndex = ::dataPool.PIDToArrayIndex(address);
	DataSet testSet;
	PoolRawAccess varToGetSize(poolId, arrayIndex, &testSet,
			PoolVariableIF::VAR_READ);
	ReturnValue_t status = testSet.read();
	if (status != RETURN_OK) {
		return INVALID_ADDRESS;
	}
	uint8_t typeSize = varToGetSize.getSizeOfType();
	if (size > varToGetSize.getSizeTillEnd()) {
		return INVALID_SIZE;
	}
	uint8_t* ptrToCopy = copyHere;
	for (; size > 0; size -= typeSize) {
		DataSet rawSet;
		PoolRawAccess variable(poolId, arrayIndex, &rawSet,
				PoolVariableIF::VAR_READ);
		status = rawSet.read();
		if (status == RETURN_OK) {
			size_t temp = 0;
			status = variable.getEntryEndianSafe(ptrToCopy, &temp, size);
			if (status != RETURN_OK) {
				return RETURN_FAILED;
			}
		} else {
			//Error reading parameter.
		}
		arrayIndex += 1;
		ptrToCopy += typeSize;
	}
	return ACTIVITY_COMPLETED;
}

ReturnValue_t DataPoolAdmin::initialize() {
	ReturnValue_t result = SystemObject::initialize();
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	result = memoryHelper.initialize(commandQueue);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	storage = objectManager->get<StorageManagerIF>(objects::IPC_STORE);
	if (storage == NULL) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	result = actionHelper.initialize(commandQueue);

	return result;
}

//mostly identical to ParameterHelper::handleParameterMessage()
ReturnValue_t DataPoolAdmin::handleParameterCommand(CommandMessage* command) {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
	switch (command->getCommand()) {
	case ParameterMessage::CMD_PARAMETER_DUMP: {
		uint8_t domain = HasParametersIF::getDomain(
				ParameterMessage::getParameterId(command));
		uint16_t parameterId = HasParametersIF::getMatrixId(
				ParameterMessage::getParameterId(command));

		DataPoolParameterWrapper wrapper;
		result = wrapper.set(domain, parameterId);

		if (result == HasReturnvaluesIF::RETURN_OK) {
			result = sendParameter(command->getSender(),
					ParameterMessage::getParameterId(command), &wrapper);
		}
	}
		break;
	case ParameterMessage::CMD_PARAMETER_LOAD: {

		uint8_t domain = HasParametersIF::getDomain(
				ParameterMessage::getParameterId(command));
		uint16_t parameterId = HasParametersIF::getMatrixId(
				ParameterMessage::getParameterId(command));
		uint8_t index = HasParametersIF::getIndex(
				ParameterMessage::getParameterId(command));

		const uint8_t *storedStream;
		size_t storedStreamSize;
		result = storage->getData(ParameterMessage::getStoreId(command),
				&storedStream, &storedStreamSize);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			break;
		}

		ParameterWrapper streamWrapper;
		result = streamWrapper.set(storedStream, storedStreamSize);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			storage->deleteData(ParameterMessage::getStoreId(command));
			break;
		}

		DataPoolParameterWrapper poolWrapper;
		result = poolWrapper.set(domain, parameterId);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			storage->deleteData(ParameterMessage::getStoreId(command));
			break;
		}

		result = poolWrapper.copyFrom(&streamWrapper, index);

		storage->deleteData(ParameterMessage::getStoreId(command));

		if (result == HasReturnvaluesIF::RETURN_OK) {
			result = sendParameter(command->getSender(),
					ParameterMessage::getParameterId(command), &poolWrapper);
		}
	}
		break;
	default:
		return HasReturnvaluesIF::RETURN_FAILED;
	}

	if (result != HasReturnvaluesIF::RETURN_OK) {
		rejectCommand(command->getSender(), result, command->getCommand());
	}

	return HasReturnvaluesIF::RETURN_OK;

}

//identical to ParameterHelper::sendParameter()
ReturnValue_t DataPoolAdmin::sendParameter(MessageQueueId_t to, uint32_t id,
		const DataPoolParameterWrapper* wrapper) {
	size_t serializedSize = wrapper->getSerializedSize();

	uint8_t *storeElement;
	store_address_t address;

	ReturnValue_t result = storage->getFreeElement(&address, serializedSize,
			&storeElement);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}

	size_t storeElementSize = 0;

	result = wrapper->serialize(&storeElement, &storeElementSize,
			serializedSize, SerializeIF::Endianness::BIG);

	if (result != HasReturnvaluesIF::RETURN_OK) {
		storage->deleteData(address);
		return result;
	}

	CommandMessage reply;

	ParameterMessage::setParameterDumpReply(&reply, id, address);

	commandQueue->sendMessage(to, &reply);

	return HasReturnvaluesIF::RETURN_OK;
}

//identical to ParameterHelper::rejectCommand()
void DataPoolAdmin::rejectCommand(MessageQueueId_t to, ReturnValue_t reason,
		Command_t initialCommand) {
	CommandMessage reply;
	reply.setReplyRejected(reason, initialCommand);
	commandQueue->sendMessage(to, &reply);
}

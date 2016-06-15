/*
 * MemoryHelper.cpp
 *
 *  Created on: 29.10.2013
 *      Author: Bastian
 */

#include <framework/globalfunctions/crc_ccitt.h>
#include <framework/memory/MemoryHelper.h>
#include <framework/memory/MemoryMessage.h>
#include <framework/objectmanager/ObjectManagerIF.h>
#include <framework/serialize/EndianSwapper.h>

MemoryHelper::MemoryHelper(HasMemoryIF* workOnThis, MessageQueue* useThisQueue) :
		workOnThis(workOnThis), queueToUse(useThisQueue), ipcStore(NULL), ipcAddress(), lastCommand(
				CommandMessage::CMD_NONE), lastSender(0), reservedSpaceInIPC(
				NULL) {
}

ReturnValue_t MemoryHelper::handleMemoryCommand(CommandMessage* message) {
	lastSender = message->getSender();
	lastCommand = message->getCommand();
	switch (lastCommand) {
	case MemoryMessage::CMD_MEMORY_DUMP:
		handleMemoryCheckOrDump(message);
		return RETURN_OK;
	case MemoryMessage::CMD_MEMORY_LOAD:
		handleMemoryLoad(message);
		return RETURN_OK;
	case MemoryMessage::CMD_MEMORY_CHECK:
		handleMemoryCheckOrDump(message);
		return RETURN_OK;
	default:
		lastCommand = CommandMessage::CMD_NONE;
		return UNKNOWN_CMD;
	}
}

ReturnValue_t MemoryHelper::initialize() {
	ipcStore = objectManager->get<StorageManagerIF>(objects::IPC_STORE);
	if (ipcStore != NULL) {
		return RETURN_OK;
	} else {
		return RETURN_FAILED;
	}
}

void MemoryHelper::completeLoad(ReturnValue_t errorCode,
		const uint8_t* dataToCopy, const uint32_t size, uint8_t* copyHere) {
	switch (errorCode) {
	case HasMemoryIF::DO_IT_MYSELF:
		return;
	case HasMemoryIF::POINTS_TO_MEMORY:
		memcpy(copyHere, dataToCopy, size);
		break;
	case HasMemoryIF::POINTS_TO_VARIABLE:
		EndianSwapper::swap(copyHere, dataToCopy, size);
		break;
	case HasMemoryIF::ACTIVITY_COMPLETED:
	case RETURN_OK:
		break;
	default:
		ipcStore->deleteData(ipcAddress);
		CommandMessage reply;
		MemoryMessage::setMemoryReplyFailed(&reply, errorCode,
				MemoryMessage::CMD_MEMORY_LOAD);
		queueToUse->sendMessage(lastSender, &reply);
		return;
	}
	//Only reached on success
	CommandMessage reply(CommandMessage::REPLY_COMMAND_OK, 0, 0);
	queueToUse->sendMessage(lastSender, &reply);
	ipcStore->deleteData(ipcAddress);
}

void MemoryHelper::completeDump(ReturnValue_t errorCode,
		const uint8_t* dataToCopy, const uint32_t size) {
	CommandMessage reply;
	MemoryMessage::setMemoryReplyFailed(&reply, errorCode, lastCommand);
	switch (errorCode) {
	case HasMemoryIF::DO_IT_MYSELF:
		return;
	case HasReturnvaluesIF::RETURN_OK:
	case HasMemoryIF::POINTS_TO_MEMORY:
	case HasMemoryIF::POINTS_TO_VARIABLE:
		//"data" must be valid pointer!
		if (errorCode == HasMemoryIF::POINTS_TO_VARIABLE) {
			EndianSwapper::swap(reservedSpaceInIPC, dataToCopy, size);
		} else {
			memcpy(reservedSpaceInIPC, dataToCopy, size);
		}
	case HasMemoryIF::ACTIVITY_COMPLETED:
		switch (lastCommand) {
		case MemoryMessage::CMD_MEMORY_DUMP: {
			MemoryMessage::setMemoryDumpReply(&reply, ipcAddress);
			break;
		}
		case MemoryMessage::CMD_MEMORY_CHECK: {
			uint16_t crc = ::Calculate_CRC(reservedSpaceInIPC, size);
			//Delete data immediately, was temporary.
			ipcStore->deleteData(ipcAddress);
			MemoryMessage::setMemoryCheckReply(&reply, crc);
			break;
		}
		default:
			//This should never happen!
			//Is it ok to send message? Otherwise: return;
			ipcStore->deleteData(ipcAddress);
			reply.setParameter(STATE_MISMATCH);
			break;
		}
		break;
	default:
		//Reply is already set to REJECTED.
		ipcStore->deleteData(ipcAddress);
		break;
	}
	if (queueToUse->sendMessage(lastSender, &reply) != RETURN_OK) {
		reply.clearCommandMessage();
	}
}

void MemoryHelper::swapMatrixCopy(uint8_t* out, const uint8_t *in,
		uint32_t totalSize, uint8_t datatypeSize) {
	if (totalSize % datatypeSize != 0){
		return;
	}

	while (totalSize > 0){
		EndianSwapper::swap(out,in,datatypeSize);
		out += datatypeSize;
		in += datatypeSize;
		totalSize -= datatypeSize;
	}
}

MemoryHelper::~MemoryHelper() {
	//Nothing to destroy
}

void MemoryHelper::handleMemoryLoad(CommandMessage* message) {
	uint32_t address = MemoryMessage::getAddress(message);
	ipcAddress = MemoryMessage::getStoreID(message);
	const uint8_t* p_data = NULL;
	uint8_t* dataPointer = NULL;
	uint32_t size = 0;
	ReturnValue_t returnCode = ipcStore->getData(ipcAddress, &p_data, &size);
	if (returnCode == RETURN_OK) {
		returnCode = workOnThis->handleMemoryLoad(address, p_data, size,
				&dataPointer);
		completeLoad(returnCode, p_data, size, dataPointer);
	} else {
		//At least inform sender.
		CommandMessage reply;
		MemoryMessage::setMemoryReplyFailed(&reply, returnCode,
				MemoryMessage::CMD_MEMORY_LOAD);
		queueToUse->sendMessage(lastSender, &reply);
	}
}

void MemoryHelper::handleMemoryCheckOrDump(CommandMessage* message) {
	uint32_t address = MemoryMessage::getAddress(message);
	uint32_t size = MemoryMessage::getLength(message);
	uint8_t* dataPointer = NULL;
	ReturnValue_t returnCode = ipcStore->getFreeElement(&ipcAddress, size,
			&reservedSpaceInIPC);
	if (returnCode == RETURN_OK) {
		returnCode = workOnThis->handleMemoryDump(address, size, &dataPointer,
				reservedSpaceInIPC);
		completeDump(returnCode, dataPointer, size);
	} else {
		CommandMessage reply;
		MemoryMessage::setMemoryReplyFailed(&reply, returnCode, lastCommand);
		queueToUse->sendMessage(lastSender, &reply);
	}
}

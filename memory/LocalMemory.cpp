/*
 * LocalMemory.cpp
 *
 *  Created on: 05.11.2013
 *      Author: Bastian
 */

#include <bsp_flp/hw_prom/HwProm.h>
#include <framework/memory/LocalMemory.h>
#include <framework/serialize/SerializeAdapter.h>

LocalMemory::LocalMemory(object_id_t setObjectId) :
		SystemObject(setObjectId), commandQueue(), memoryHelper(this,
				&commandQueue) {
}

ReturnValue_t LocalMemory::performOperation() {
	ReturnValue_t handleResult;
	CommandMessage message;
	for (ReturnValue_t result = commandQueue.receiveMessage(&message);
			result == HasReturnvaluesIF::RETURN_OK;
			result = commandQueue.receiveMessage(&message)) {
		handleResult = memoryHelper.handleMemoryCommand(&message);
		if (handleResult != HasReturnvaluesIF::RETURN_OK) {
			message.setToUnknownCommand(message.getCommand());
			commandQueue.reply(&message);
		}
	}
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t LocalMemory::handleMemoryLoad(uint32_t address,
		const uint8_t* data, uint32_t size, uint8_t** dataPointer) {
	ReturnValue_t result = checkWriteAccess(address, size);
	if (result == HasReturnvaluesIF::RETURN_OK) {
		uint32_t value = 0;
		for (uint32_t temp_address = address; temp_address < (address + size);
				temp_address += 4, data += 4) {
			value = (data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3];
			*((uint32_t*) temp_address) = value;
		}
	}
	return result;
}

ReturnValue_t LocalMemory::handleMemoryDump(uint32_t address, uint32_t size,
		uint8_t** dataPointer, uint8_t* dumpTarget) {
	*dataPointer = (uint8_t*) address;
	return POINTS_TO_MEMORY;
}

ReturnValue_t LocalMemory::initialize() {
	return memoryHelper.initialize();
}

MessageQueueId_t LocalMemory::getCommandQueue() const {
	return commandQueue.getId();
}

ReturnValue_t LocalMemory::checkWriteAccess(uint32_t address, uint32_t size) {

	if ((address % 4) != 0) {
		return UNALIGNED_ACCESS;
	}

	if ((size % 4) != 0) {
		return INVALID_SIZE;
	}

	if (address < 0x40000000) {
		HwProm prom(false);
		if (prom.getPromWriteEnabled() != HwProm::WRITE_ENABLED) {
			return WRITE_PROTECTED;
		}
	}

	return HasReturnvaluesIF::RETURN_OK;
}


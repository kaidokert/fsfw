/*
 * DataPoolAdmin.cpp
 *
 *  Created on: 05.12.2013
 *      Author: baetz
 */



#include <framework/datapool/DataPool.h>
#include <framework/datapool/DataPoolAdmin.h>
#include <framework/datapool/DataSet.h>
#include <framework/datapool/PoolRawAccess.h>
#include <framework/ipc/CommandMessage.h>


DataPoolAdmin::DataPoolAdmin(object_id_t objectId ) : SystemObject(objectId), commandQueue(), memoryHelper(this, &commandQueue){
}

ReturnValue_t DataPoolAdmin::performOperation() {
	handleCommand();
	return RETURN_OK;
}

MessageQueueId_t DataPoolAdmin::getCommandQueue() const {
	return commandQueue.getId();
}

void DataPoolAdmin::handleCommand() {
	CommandMessage command;
	ReturnValue_t result = commandQueue.receiveMessage(&command);
	if (result != RETURN_OK) {
		return;
	}
	result = memoryHelper.handleMemoryCommand(&command);
	if (result != RETURN_OK) {
		command.setToUnknownCommand(command.getCommand());
		commandQueue.reply( &command );
	}
}

ReturnValue_t DataPoolAdmin::handleMemoryLoad(uint32_t address,
		const uint8_t* data, uint32_t size, uint8_t** dataPointer) {
	uint32_t poolId = ::dataPool.PIDToDataPoolId( address );
	uint8_t arrayIndex = ::dataPool.PIDToArrayIndex( address );
	DataSet testSet;
	PoolRawAccess varToGetSize( poolId, arrayIndex, &testSet, PoolVariableIF::VAR_READ );
	ReturnValue_t status = testSet.read();
	if (status != RETURN_OK) {
		return INVALID_ADDRESS;
	}
	uint8_t typeSize = varToGetSize.getSizeOfType();
	if ( size > varToGetSize.getSizeTillEnd() ) {
		return INVALID_SIZE;
	}
	const uint8_t* readPosition = data;
	for ( ; size > 0; size -= typeSize ) {
		DataSet rawSet;
		PoolRawAccess variable( poolId, arrayIndex, &rawSet, PoolVariableIF::VAR_READ_WRITE );
		status = rawSet.read();
		if (status == RETURN_OK) {
			status = variable.setEntryFromBigEndian( readPosition, typeSize );
			if (status == RETURN_OK) {
				status = rawSet.commit(PoolVariableIF::VALID);
			}
		}
		arrayIndex += 1;
		readPosition += typeSize;
	}
	return ACTIVITY_COMPLETED;
}

ReturnValue_t DataPoolAdmin::handleMemoryDump(uint32_t address, uint32_t size,
		uint8_t** dataPointer, uint8_t* copyHere) {
	uint32_t poolId = ::dataPool.PIDToDataPoolId( address );
	uint8_t arrayIndex = ::dataPool.PIDToArrayIndex( address );
	DataSet testSet;
	PoolRawAccess varToGetSize( poolId, arrayIndex, &testSet, PoolVariableIF::VAR_READ );
	ReturnValue_t status = testSet.read();
	if (status != RETURN_OK) {
		return INVALID_ADDRESS;
	}
	uint8_t typeSize = varToGetSize.getSizeOfType();
	if ( size > varToGetSize.getSizeTillEnd() ) {
		return INVALID_SIZE;
	}
	uint8_t* ptrToCopy = copyHere;
	for ( ; size > 0; size -= typeSize ) {
		DataSet rawSet;
		PoolRawAccess variable( poolId, arrayIndex, &rawSet, PoolVariableIF::VAR_READ );
		status = rawSet.read();
		if (status == RETURN_OK) {
			uint32_t temp = 0;
			status = variable.getEntryEndianSafe( ptrToCopy, &temp,  size);
			if ( status != RETURN_OK ) {
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
	if (memoryHelper.initialize() == RETURN_OK) {
		return SystemObject::initialize();
	}
	return RETURN_FAILED;
}

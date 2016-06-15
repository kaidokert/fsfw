/*
 * MemoryHelper.h
 *
 *  Created on: 29.10.2013
 *      Author: Bastian
 */

#ifndef MEMORYHELPER_H_
#define MEMORYHELPER_H_
#include <framework/ipc/CommandMessage.h>
#include <framework/ipc/MessageQueue.h>
#include <framework/memory/AcceptsMemoryMessagesIF.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/storagemanager/StorageManagerIF.h>

class MemoryHelper : public HasReturnvaluesIF {
public:
	static const uint8_t INTERFACE_ID = MEMORY_HELPER;
	static const ReturnValue_t UNKNOWN_CMD = MAKE_RETURN_CODE(0xE0);
	static const ReturnValue_t INVALID_ADDRESS = MAKE_RETURN_CODE(0xE1);
	static const ReturnValue_t INVALID_SIZE = MAKE_RETURN_CODE(0xE2);
	static const ReturnValue_t STATE_MISMATCH = MAKE_RETURN_CODE(0xE3);
private:
	HasMemoryIF* workOnThis;
	MessageQueue* queueToUse;
	StorageManagerIF* ipcStore;
	store_address_t ipcAddress;
	Command_t lastCommand;
	MessageQueueId_t lastSender;
	uint8_t* reservedSpaceInIPC;
	void handleMemoryLoad(CommandMessage* message);
	void handleMemoryCheckOrDump(CommandMessage* message);
public:
	ReturnValue_t handleMemoryCommand(CommandMessage* message);
	void completeLoad( ReturnValue_t errorCode, const uint8_t* dataToCopy = NULL, const uint32_t size = 0, uint8_t* copyHere = NULL );
	void completeDump(  ReturnValue_t errorCode, const uint8_t* dataToCopy = NULL, const uint32_t size = 0);
	void swapMatrixCopy( uint8_t *out, const uint8_t *in, uint32_t totalSize, uint8_t datatypeSize);
	ReturnValue_t initialize();
	MemoryHelper( HasMemoryIF* workOnThis, MessageQueue* useThisQueue );
	~MemoryHelper();
};
#endif /* MEMORYHELPER_H_ */

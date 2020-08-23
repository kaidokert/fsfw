#ifndef FRAMEWORK_MEMORY_MEMORYHELPER_H_
#define FRAMEWORK_MEMORY_MEMORYHELPER_H_
#include "../ipc/CommandMessage.h"
#include "../memory/AcceptsMemoryMessagesIF.h"
#include "../returnvalues/HasReturnvaluesIF.h"
#include "../storagemanager/StorageManagerIF.h"
#include "../ipc/MessageQueueIF.h"

class MemoryHelper : public HasReturnvaluesIF {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::MEMORY_HELPER;
	static const ReturnValue_t UNKNOWN_CMD = MAKE_RETURN_CODE(0xE0);
	static const ReturnValue_t INVALID_ADDRESS = MAKE_RETURN_CODE(0xE1);
	static const ReturnValue_t INVALID_SIZE = MAKE_RETURN_CODE(0xE2);
	static const ReturnValue_t STATE_MISMATCH = MAKE_RETURN_CODE(0xE3);
private:
	HasMemoryIF* workOnThis;
	MessageQueueIF* queueToUse;
	StorageManagerIF* ipcStore = nullptr;
	store_address_t ipcAddress;
	Command_t lastCommand;
	MessageQueueId_t lastSender = MessageQueueIF::NO_QUEUE;
	uint8_t* reservedSpaceInIPC = nullptr;
	bool busy;
	void handleMemoryLoad(CommandMessage* message);
	void handleMemoryCheckOrDump(CommandMessage* message);
	ReturnValue_t initialize();
public:
	ReturnValue_t handleMemoryCommand(CommandMessage* message);
	void completeLoad( ReturnValue_t errorCode, const uint8_t* dataToCopy = NULL, const uint32_t size = 0, uint8_t* copyHere = NULL );
	void completeDump(  ReturnValue_t errorCode, const uint8_t* dataToCopy = NULL, const uint32_t size = 0);
	void swapMatrixCopy( uint8_t *out, const uint8_t *in, uint32_t totalSize, uint8_t datatypeSize);
	ReturnValue_t initialize(MessageQueueIF* queueToUse_);
	MemoryHelper( HasMemoryIF* workOnThis, MessageQueueIF* useThisQueue );
	~MemoryHelper();
};
#endif /* MEMORYHELPER_H_ */

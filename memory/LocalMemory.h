#ifndef LOCALMEMORY_H_
#define LOCALMEMORY_H_

#include <framework/memory/HasMemoryIF.h>
#include <framework/memory/MemoryHelper.h>
#include <framework/objectmanager/SystemObject.h>
#include <framework/tasks/ExecutableObjectIF.h>
#include <framework/ipc/MessageQueueIF.h>
#include <list>
class LocalMemory: public AcceptsMemoryMessagesIF,
		public ExecutableObjectIF,
		public SystemObject {
private:
	MessageQueueIF* commandQueue;
	MemoryHelper memoryHelper;
	ReturnValue_t checkWriteAccess(uint32_t address, uint32_t size);
public:
	LocalMemory(object_id_t setObjectId);
	~LocalMemory();
	ReturnValue_t performOperation(uint8_t opCode);
	ReturnValue_t initialize();
	MessageQueueId_t getCommandQueue() const;
	ReturnValue_t handleMemoryLoad(uint32_t address, const uint8_t* data,
			uint32_t size, uint8_t** dataPointer);
	ReturnValue_t handleMemoryDump(uint32_t address, uint32_t size,
			uint8_t** dataPointer, uint8_t* dumpTarget);
};

#endif /* LOCALMEMORY_H_ */

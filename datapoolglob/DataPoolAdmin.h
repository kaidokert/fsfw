#ifndef DATAPOOLADMIN_H_
#define DATAPOOLADMIN_H_

#include <framework/objectmanager/SystemObject.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/tasks/ExecutableObjectIF.h>
#include <framework/action/HasActionsIF.h>
#include <framework/ipc/MessageQueueIF.h>
#include <framework/parameters/ReceivesParameterMessagesIF.h>

#include <framework/memory/MemoryHelper.h>
#include <framework/action/SimpleActionHelper.h>
#include <framework/datapoolglob/DataPoolParameterWrapper.h>

class DataPoolAdmin: public HasActionsIF,
		public ExecutableObjectIF,
		public AcceptsMemoryMessagesIF,
		public HasReturnvaluesIF,
		public ReceivesParameterMessagesIF,
		public SystemObject {
public:
	static const ActionId_t SET_VALIDITY = 1;

	DataPoolAdmin(object_id_t objectId);

	~DataPoolAdmin();

	ReturnValue_t performOperation(uint8_t opCode);

	MessageQueueId_t getCommandQueue() const;

	ReturnValue_t handleMemoryLoad(uint32_t address, const uint8_t* data,
			size_t size, uint8_t** dataPointer);
	ReturnValue_t handleMemoryDump(uint32_t address, size_t size,
			uint8_t** dataPointer, uint8_t* copyHere);

	ReturnValue_t executeAction(ActionId_t actionId,
			MessageQueueId_t commandedBy, const uint8_t* data, size_t size);

	//not implemented as ParameterHelper is no used
	ReturnValue_t getParameter(uint8_t domainId, uint16_t parameterId,
			ParameterWrapper *parameterWrapper,
			const ParameterWrapper *newValues, uint16_t startAtIndex);

	ReturnValue_t initialize();
private:
	StorageManagerIF *storage;
	MessageQueueIF* commandQueue;
	MemoryHelper memoryHelper;
	SimpleActionHelper actionHelper;
	void handleCommand();
	ReturnValue_t handleParameterCommand(CommandMessage *command);
	ReturnValue_t sendParameter(MessageQueueId_t to, uint32_t id,
			const DataPoolParameterWrapper* wrapper);
	void rejectCommand(MessageQueueId_t to, ReturnValue_t reason,
			Command_t initialCommand);
};

#endif /* DATAPOOLADMIN_H_ */

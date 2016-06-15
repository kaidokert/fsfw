/*
 * DataPoolAdmin.h
 *
 *  Created on: 05.12.2013
 *      Author: baetz
 */

#ifndef DATAPOOLADMIN_H_
#define DATAPOOLADMIN_H_

#include <framework/ipc/MessageQueue.h>
#include <framework/memory/MemoryHelper.h>
#include <framework/objectmanager/SystemObject.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/tasks/ExecutableObjectIF.h>

class DataPoolAdmin : public ExecutableObjectIF, public AcceptsMemoryMessagesIF, public HasReturnvaluesIF, public SystemObject {
private:
	MessageQueue commandQueue;
	MemoryHelper memoryHelper;
	void handleCommand();
public:
	DataPoolAdmin(object_id_t objectId);

	ReturnValue_t performOperation();

	MessageQueueId_t getCommandQueue() const;

	ReturnValue_t handleMemoryLoad(uint32_t address, const uint8_t* data, uint32_t size, uint8_t** dataPointer);
	ReturnValue_t handleMemoryDump(uint32_t address, uint32_t size, uint8_t** dataPointer, uint8_t* copyHere );
	ReturnValue_t initialize();
};


#endif /* DATAPOOLADMIN_H_ */

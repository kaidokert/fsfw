#include "../objectmanager/SystemObjectIF.h"
#include "../tasks/FixedSequenceSlot.h"
#include <cstddef>

FixedSequenceSlot::FixedSequenceSlot(object_id_t handlerId, uint32_t setTime,
		int8_t setSequenceId, PeriodicTaskIF* executingTask) :
		pollingTimeMs(setTime), opcode(setSequenceId) {
	handler = objectManager->get<ExecutableObjectIF>(handlerId);
	if(executingTask != nullptr) {
	    handler->setTaskIF(executingTask);
	}
	handler->initializeAfterTaskCreation();
}

FixedSequenceSlot::~FixedSequenceSlot() {}


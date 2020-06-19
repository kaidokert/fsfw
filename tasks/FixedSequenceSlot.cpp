#include <framework/objectmanager/SystemObjectIF.h>
#include <framework/tasks/FixedSequenceSlot.h>
#include <cstddef>

FixedSequenceSlot::FixedSequenceSlot(object_id_t handlerId, uint32_t setTime,
		int8_t setSequenceId, PeriodicTaskIF* executingTask) :
		pollingTimeMs(setTime), opcode(setSequenceId) {
	handler = objectManager->get<ExecutableObjectIF>(handlerId);
	if(executingTask != nullptr) {
	    handler->setTaskIF(executingTask);
	}
}

FixedSequenceSlot::~FixedSequenceSlot() {}


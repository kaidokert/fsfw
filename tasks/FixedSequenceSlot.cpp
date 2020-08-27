#include "FixedSequenceSlot.h"
#include "../objectmanager/SystemObjectIF.h"
#include <cstddef>

FixedSequenceSlot::FixedSequenceSlot(object_id_t handlerId, uint32_t setTime,
		int8_t setSequenceId, ExecutableObjectIF* executableObject,
		PeriodicTaskIF* executingTask) :
		pollingTimeMs(setTime), opcode(setSequenceId) {
    if(executableObject == nullptr) {
        return;
    }
    this->executableObject = executableObject;
    this->executableObject->setTaskIF(executingTask);
}

FixedSequenceSlot::~FixedSequenceSlot() {}


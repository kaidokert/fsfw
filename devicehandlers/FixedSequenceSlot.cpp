/**
 * @file	PollingSlot.cpp
 * @brief	This file defines the PollingSlot class.
 * @date	19.12.2012
 * @author	baetz
 */

#include <framework/devicehandlers/FixedSequenceSlot.h>
#include <framework/objectmanager/SystemObjectIF.h>
#include <cstddef>

FixedSequenceSlot::FixedSequenceSlot(object_id_t handlerId, uint32_t setTime,
		int8_t setSequenceId, PeriodicTaskIF* executingTask) :
		handler(NULL), pollingTimeMs(setTime), opcode(setSequenceId) {
	handler = objectManager->get<ExecutableObjectIF>(handlerId);
	handler->setTaskIF(executingTask);
}

FixedSequenceSlot::~FixedSequenceSlot() {
}


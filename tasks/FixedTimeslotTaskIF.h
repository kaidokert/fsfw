#ifndef FRAMEWORK_TASKS_FIXEDTIMESLOTTASKIF_H_
#define FRAMEWORK_TASKS_FIXEDTIMESLOTTASKIF_H_

#include "../objectmanager/ObjectManagerIF.h"
#include "PeriodicTaskIF.h"

/**
 * Following the same principle as the base class IF. This is the interface for a Fixed timeslot task
 */
class FixedTimeslotTaskIF : public PeriodicTaskIF {
public:
	virtual ~FixedTimeslotTaskIF() {}
	virtual ReturnValue_t addSlot(object_id_t componentId, uint32_t slotTimeMs, int8_t executionStep) = 0;
	virtual ReturnValue_t checkSequence() const = 0;
};



#endif /* FRAMEWORK_TASKS_FIXEDTIMESLOTTASKIF_H_ */

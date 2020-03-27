#ifndef FRAMEWORK_TASKS_FIXEDTIMESLOTTASKIF_H_
#define FRAMEWORK_TASKS_FIXEDTIMESLOTTASKIF_H_

#include <framework/objectmanager/ObjectManagerIF.h>
#include <framework/tasks/PeriodicTaskIF.h>

/**
 * @brief Following the same principle as the base class IF.
 *        This is the interface for a Fixed timeslot task
 */
class FixedTimeslotTaskIF : public PeriodicTaskIF {
public:
	virtual ~FixedTimeslotTaskIF() {}

	virtual ReturnValue_t addSlot(object_id_t componentId,
			uint32_t slotTimeMs, int8_t executionStep) = 0;
	virtual ReturnValue_t checkSequence() const = 0;
};



#endif /* FRAMEWORK_TASKS_FIXEDTIMESLOTTASKIF_H_ */

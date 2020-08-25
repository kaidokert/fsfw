/**
 * @file	FixedSequenceSlot.h
 * @brief	This file defines the PollingSlot class.
 * @date	19.12.2012
 * @author	baetz
 */

#ifndef FIXEDSEQUENCESLOT_H_
#define FIXEDSEQUENCESLOT_H_

#include "../objectmanager/ObjectManagerIF.h"
#include "../tasks/ExecutableObjectIF.h"
class PeriodicTaskIF;

/**
 * @brief	This class is the representation of a single polling sequence table entry.
 *
 * @details	The PollingSlot class is the representation of a single polling
 * 			sequence table entry.
 */
class FixedSequenceSlot {
public:
	FixedSequenceSlot( object_id_t handlerId, uint32_t setTimeMs,
			int8_t setSequenceId, PeriodicTaskIF* executingTask );
	virtual ~FixedSequenceSlot();

	/**
	 * @brief	Handler identifies which device handler object is executed in this slot.
	 */
	ExecutableObjectIF*	handler;

	/**
	 * @brief	This attribute defines when a device handler object is executed.
	 *
	 * @details	The pollingTime attribute identifies the time the handler is executed in ms.
	 * 			It must be smaller than the period length of the polling sequence.
	 */
	uint32_t	pollingTimeMs;

	/**
	 * @brief	This value defines the type of device communication.
	 *
	 * @details	The state of this value decides what communication routine is
	 * 			called in the PST executable or the device handler object.
	 */
	uint8_t			opcode;

	/**
	 * @brief 	Operator overload for the comparison operator to
	 * 			allow sorting by polling time.
	 * @param fixedSequenceSlot
	 * @return
	 */
	bool operator <(const FixedSequenceSlot & fixedSequenceSlot) const {
		return pollingTimeMs < fixedSequenceSlot.pollingTimeMs;
	}
};


#endif /* FIXEDSEQUENCESLOT_H_ */

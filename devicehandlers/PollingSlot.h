/**
 * @file	PollingSlot.h
 * @brief	This file defines the PollingSlot class.
 * @date	19.12.2012
 * @author	baetz
 */

#ifndef POLLINGSLOT_H_
#define POLLINGSLOT_H_

#include <framework/devicehandlers/PollingSequenceExecutableIF.h>
#include <framework/objectmanager/ObjectManagerIF.h>
#include <framework/osal/OSAL.h>

class PollingSequence;

/**
 * \brief	This class is the representation of a single polling sequence table entry.
 *
 * \details	The PollingSlot class is the representation of a single polling sequence table entry.
 * 			It contains three attributes and a debug method to print its content.
 */
class PollingSlot {
	friend class PollingSequence;
	friend class PollingTask;
	friend ReturnValue_t pollingSequenceInitFunction(PollingSequence *thisSequence);
	friend ReturnValue_t payloadSequenceInitFunction(PollingSequence *thisSequence);
protected:
	/**
	 * \brief	\c handler identifies which device handler object is executed in this slot.
	 */
	PollingSequenceExecutableIF*	handler;

	/**
	 * \brief	This attribute defines when a device handler object is executed.
	 *
	 * \details	The pollingTime attribute identifies the time the handler is executed in clock ticks. It must be
	 * 			smaller than the period length of the polling sequence, what is ensured by automated calculation
	 * 			from a database.
	 */
	Interval_t	pollingTime;

	/**
	 * \brief	This value defines the type of device communication.
	 *
	 * \details	The state of this value decides what communication routine is called in the PST executable or the device handler object.
	 */
	uint8_t			opcode;

public:
			PollingSlot( object_id_t handlerId, Interval_t setTime, int8_t setSequenceId );
	virtual ~PollingSlot();

	/**
	 * \brief	This is a small debug method to print the PollingSlot's content to a debug interface.
	 */
	void	print() const;
};


#endif /* POLLINGSLOT_H_ */

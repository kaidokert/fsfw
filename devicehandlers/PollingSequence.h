/**
 * @file	PollingSequence.h
 * @brief	This file defines the PollingSequence class.
 * @date	19.12.2012
 * @author	baetz
 */

#ifndef POLLINGSEQUENCE_H_
#define POLLINGSEQUENCE_H_

#include <framework/devicehandlers/PollingSequenceExecutableIF.h>
#include <framework/devicehandlers/PollingSlot.h>
#include <framework/objectmanager/SystemObject.h>
#include <framework/osal/OSAL.h>
#include<list>


/**
 * \brief	This class is the representation of a Polling Sequence Table in software.
 *
 * \details	The PollingSequence object maintains the dynamic execution of device handler objects.
 * 			The main idea is to create a list of device handlers, to announce all handlers to the
 * 			polling sequence and to maintain a list of polling slot objects. This slot list represents the
 * 			Polling Sequence Table in software. Each polling slot contains information to indicate when and
 * 			which device handler shall be executed within a given polling period.
 * 			The sequence is then executed by iterating through this slot list.
 * 			Handlers are invoking by calling a certain function stored in the handler list.
 */
class PollingSequence : public SystemObject {
	friend class PollingTask;
	friend ReturnValue_t pollingSequenceInitFunction(PollingSequence *thisSequence);
	friend ReturnValue_t payloadSequenceInitFunction(PollingSequence *thisSequence);
protected:

	/**
	 * \brief	This list contains all OPUSPollingSlot objects, defining order and execution time of the
	 * 			device handler objects.
	 *
	 * \details	The slot list is a std:list object that contains all created OPUSPollingSlot instances.
	 * 			They are NOT ordered automatically, so by adding entries, the correct order needs to be ensured.
	 * 			By iterating through this list the polling sequence is executed. Two entries with identical
	 * 			polling times are executed immediately one after another.
	 */
	std::list<PollingSlot*> slotList;

	/**
	 * \brief	An iterator that indicates the current polling slot to execute.
	 *
	 * \details	This is an iterator for slotList and always points to the polling slot which is executed next.
	 */
	std::list<PollingSlot*>::iterator current;

	/**
	 * \brief	The period of the Polling Sequence Table in clock ticks.
	 *
	 * \details	This attribute is set within the constructor, defining the main period length of the
	 * 			Polling Sequence Table. The length is expressed in clock ticks.
	 *
	 */
	Interval_t length;

	/**
	* \brief	The init function passed by the ctor
	*
	* \details	This function will be called during initialize()
	*
	*/
	ReturnValue_t (*initFunction)(PollingSequence *thisSequence);

public:

	/**
	 * \brief	The constructor of the PollingSequence object.
	 *
	 * \details	The constructor takes two arguments, the period length and the init function.
	 *
	 * \param	setLength	The period length, expressed in clock ticks.
	 */
	PollingSequence( object_id_t setObjectId, Interval_t setLength, ReturnValue_t (*initFunction)(PollingSequence *thisSequence) );

	/**
	 * \brief	The destructor of the PollingSequence object.
	 *
	 * \details	The destructor frees all allocated memory by iterating through
	 * 			handlerMap and slotList and deleting all allocated resources.
	 */
	virtual ~PollingSequence();

	/**
	 * \brief	This is a method to add an OPUSPollingSlot object to slotList.
	 *
	 * \details	Here, a polling slot object is added to the slot list. It is appended
	 * 			to the end of the list. The list is currently NOT reordered.
	 * 			Afterwards, the iterator current is set to the beginning of the list.
	 *
	 * \param	setSlot This is a pointer to a OPUSPollingSlot object.
	 */
	void addSlot( PollingSlot* setSlot );

	/**
	 * Checks if the current slot shall be executed immediately after the one before.
	 * This allows to distinguish between grouped and not grouped handlers.
	 * @return 	- @c true if the slot has the same polling time as the previous
	 * 			- @c false else
	 */
	bool slotFollowsImmediately();

	/**
	 * \brief	This method returns the time until the next device handler object is invoked.
	 *
	 * \details	This method is vitally important for the operation of the PST. By fetching the polling time
	 * 			of the current slot and that of the next one (or the first one, if the list end is reached)
	 * 			it calculates and returns the interval in clock ticks within which the handler execution
	 * 			shall take place.
	 */
	Interval_t getInterval();

	/**
	 * \brief	This method returns the length of this PollingSequence instance.
	 */
	Interval_t getLength();

	/**
	 * \brief	The method to execute the device handler entered in the current OPUSPollingSlot object.
	 *
	 * \details	Within this method the device handler object to be executed is chosen by looking up the
	 * 			handler address of the current slot in the handlerMap. Either the device handler's
	 * 			talkToInterface or its listenToInterface method is invoked, depending on the isTalking flag
	 * 			of the polling slot. After execution the iterator current is increased or, by reaching the
	 * 			end of slotList, reset to the beginning.
	 */
	void pollAndAdvance();

	/**
	 * \brief	This is a method to print debug output.
	 *
	 * \details	print is a simple debug method to print the whole polling sequence to the debug interface.
	 * 			It iterates through slotList and calls all print()} functions of the announced polling slot
	 * 			instances.
	 *
	 */
	void print();

	ReturnValue_t initialize();
	//std::string getObjectTypeAsString();
};


#endif /* POLLINGSEQUENCE_H_ */

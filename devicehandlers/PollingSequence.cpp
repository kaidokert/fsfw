/**
 * @file	PollingSequence.cpp
 * @brief	This file defines the PollingSequence class.
 * @date	19.12.2012
 * @author	baetz
 */

#include <framework/devicehandlers/PollingSequence.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>


uint32_t PollingSequenceExecutableIF::pollingSequenceLengthMs = 0;
uint32_t PollingSequenceExecutableIF::payloadSequenceLengthMs = 0;

PollingSequence::PollingSequence(object_id_t setObjectId, Interval_t setLength, ReturnValue_t (*initFunction)(PollingSequence *thisSequence)) : SystemObject( setObjectId ),
			initFunction(initFunction){
	this->length = setLength;
//	PollingSequenceExecutableIF::pollingSequenceLengthMs = (setLength * 1000)
//			/ OSAL::getTicksPerSecond();
	current = slotList.begin();
}

PollingSequence::~PollingSequence() {
	std::list<PollingSlot*>::iterator slotIt;
	//Iterate through slotList and delete all entries.
	slotIt = this->slotList.begin();
	while (slotIt != this->slotList.end()) {
		delete (*slotIt);
		slotIt++;
	}
}

void PollingSequence::addSlot(PollingSlot* setSlot) {
	//The slot is added to the end of the list.
	this->slotList.push_back(setSlot);
	this->current = slotList.begin();
}

void PollingSequence::pollAndAdvance() {
//	(*this->current)->print();
	(*this->current)->handler->performInPST( (*this->current)->opcode );
//	if (returnValue != RETURN_OK) {
//		this->sendErrorMessage( returnValue );
//	}
	//Increment the polling Sequence iterator
	this->current++;
	//Set it to the beginning, if the list's end is reached.
	if (this->current == this->slotList.end()) {
		this->current = this->slotList.begin();
	}
}

Interval_t PollingSequence::getInterval() {
	Interval_t oldTime;
	std::list<PollingSlot*>::iterator it;
	it = this->current;
	// Get the pollingTime of the current slot object.
	oldTime = (*it)->pollingTime;
	// Advance to the next object.
	it++;
	// Find the next interval which is not 0.
	while ( it != slotList.end() ) {
		if ( oldTime != (*it)->pollingTime ) {
			return (*it)->pollingTime - oldTime;
		} else {
			it++;
		}
	}
	// If the list end is reached (this is definitely an interval != 0),
	// the interval is calculated by subtracting the remaining time of the PST
	// and adding the start time of the first handler in the list.
	it = slotList.begin();
	return this->length - oldTime + (*it)->pollingTime;
}

bool PollingSequence::slotFollowsImmediately() {
	Interval_t currentTime = (*current)->pollingTime;
	std::list<PollingSlot*>::iterator it;
	it = this->current;
	// Get the pollingTime of the current slot object.
	if ( it == slotList.begin() ) return false;
	it--;
	if ( (*it)->pollingTime == currentTime ) {
		return true;
	} else {
		return false;
	}
}

Interval_t PollingSequence::getLength() {
	return this->length;
}

void PollingSequence::print() {
	debug << "Polling sequence contains:" << std::endl;

	//Iterate through slotList and start all PollingSlot::print() methods.
	do {
		(*current)->print();
		current++;
	} while (current != slotList.end());
	current = slotList.begin();
}

//std::string PollingSequence::getObjectTypeAsString() {
//	return "PollingSequence";
//}

ReturnValue_t PollingSequence::initialize() {
	ReturnValue_t result = SystemObject::initialize();
	if (result != HasReturnvaluesIF::RETURN_OK){
		return result;
	}
	return initFunction(this);
}

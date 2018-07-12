#include <framework/devicehandlers/FixedSlotSequence.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

FixedSlotSequence::FixedSlotSequence(uint32_t setLengthMs) :
		lengthMs(setLengthMs) {
	current = slotList.begin();
}

FixedSlotSequence::~FixedSlotSequence() {
	std::list<FixedSequenceSlot*>::iterator slotIt;
	//Iterate through slotList and delete all entries.
	slotIt = this->slotList.begin();
	while (slotIt != this->slotList.end()) {
		delete (*slotIt);
		slotIt++;
	}
}

void FixedSlotSequence::executeAndAdvance() {
//	(*this->current)->print();
	(*this->current)->handler->performOperation((*this->current)->opcode);
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

uint32_t FixedSlotSequence::getIntervalMs() {
	uint32_t oldTime;
	std::list<FixedSequenceSlot*>::iterator it;
	it = current;
	// Get the pollingTimeMs of the current slot object.
	oldTime = (*it)->pollingTimeMs;
	// Advance to the next object.
	it++;
	// Find the next interval which is not 0.
	while (it != slotList.end()) {
		if (oldTime != (*it)->pollingTimeMs) {
			return (*it)->pollingTimeMs - oldTime;
		} else {
			it++;
		}
	}
	// If the list end is reached (this is definitely an interval != 0),
	// the interval is calculated by subtracting the remaining time of the PST
	// and adding the start time of the first handler in the list.
	it = slotList.begin();
	return lengthMs - oldTime + (*it)->pollingTimeMs;
}

bool FixedSlotSequence::slotFollowsImmediately() {
	uint32_t currentTime = (*current)->pollingTimeMs;
	std::list<FixedSequenceSlot*>::iterator it;
	it = this->current;
	// Get the pollingTimeMs of the current slot object.
	if (it == slotList.begin())
		return false;
	it--;
	if ((*it)->pollingTimeMs == currentTime) {
		return true;
	} else {
		return false;
	}
}

uint32_t FixedSlotSequence::getLengthMs() const {
	return this->lengthMs;
}

ReturnValue_t FixedSlotSequence::checkSequence() const {
	//Iterate through slotList and check successful creation. Checks if timing is ok (must be ascending) and if all handlers were found.
	auto slotIt = slotList.begin();
	uint32_t count = 0;
	uint32_t time = 0;
	while (slotIt != slotList.end()) {
		if ((*slotIt)->handler == NULL) {
			error << "FixedSlotSequene::initialize: ObjectId does not exist!"
					<< std::endl;
			count++;
		} else if ((*slotIt)->pollingTimeMs < time) {
			error << "FixedSlotSequence::initialize: Time: "
					<< (*slotIt)->pollingTimeMs
					<< " is smaller than previous with " << time << std::endl;
			count++;
		} else {
			//All ok, print slot.
//			(*slotIt)->print();
		}
		time = (*slotIt)->pollingTimeMs;
		slotIt++;
	}
	if (count > 0) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

void FixedSlotSequence::addSlot(object_id_t componentId, uint32_t slotTimeMs,
		int8_t executionStep, PeriodicTaskIF* executingTask) {
	this->slotList.push_back(
			new FixedSequenceSlot(componentId, slotTimeMs, executionStep,
					executingTask));
	this->current = slotList.begin();
}

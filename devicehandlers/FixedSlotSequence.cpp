#include <framework/devicehandlers/FixedSlotSequence.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

FixedSlotSequence::FixedSlotSequence(uint32_t setLengthMs) :
		slotLengthMs(setLengthMs) {
	current = slotList.begin();
}

FixedSlotSequence::~FixedSlotSequence() {
	// This should call the destructor on each list entry.
	slotList.clear();
//	SlotListIter slotListIter = this->slotList.begin();
//	//Iterate through slotList and delete all entries.
//	while (slotListIter != this->slotList.end()) {
//		delete (*slotIt);
//		slotIt++;
//	}
}

void FixedSlotSequence::executeAndAdvance() {
	current->handler->performOperation(current->opcode);
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

uint32_t FixedSlotSequence::getIntervalToNextSlotMs() {
	uint32_t oldTime;
	SlotListIter slotListIter = current;
	// Get the pollingTimeMs of the current slot object.
	oldTime = slotListIter->pollingTimeMs;
	// Advance to the next object.
	slotListIter++;
	// Find the next interval which is not 0.
	while (slotListIter != slotList.end()) {
		if (oldTime != slotListIter->pollingTimeMs) {
			return slotListIter->pollingTimeMs - oldTime;
		} else {
			slotListIter++;
		}
	}
	// If the list end is reached (this is definitely an interval != 0),
	// the interval is calculated by subtracting the remaining time of the PST
	// and adding the start time of the first handler in the list.
	slotListIter = slotList.begin();
	return slotLengthMs - oldTime + slotListIter->pollingTimeMs;
}

uint32_t FixedSlotSequence::getIntervalToPreviousSlotMs() {
	uint32_t currentTime;
	SlotListIter slotListIter = current;
	// Get the pollingTimeMs of the current slot object.
	currentTime = slotListIter->pollingTimeMs;

	//if it is the first slot, calculate difference to last slot
	if (slotListIter == slotList.begin()){
		return slotLengthMs - (--slotList.end())->pollingTimeMs + currentTime;
	}
	// get previous slot
	slotListIter--;

	return currentTime - slotListIter->pollingTimeMs;
}

bool FixedSlotSequence::slotFollowsImmediately() {
	uint32_t currentTime = current->pollingTimeMs;
	SlotListIter fixedSequenceIter = this->current;
	// Get the pollingTimeMs of the current slot object.
	if (fixedSequenceIter == slotList.begin())
		return false;
	fixedSequenceIter--;
	if (fixedSequenceIter->pollingTimeMs == currentTime) {
		return true;
	} else {
		return false;
	}
}

uint32_t FixedSlotSequence::getLengthMs() const {
	return this->slotLengthMs;
}

ReturnValue_t FixedSlotSequence::checkSequence() const {
	// Iterate through slotList and check successful creation.
	// Checks if timing is ok (must be ascending) and if all handlers were found.
	auto slotIt = slotList.begin();
	uint32_t count = 0;
	uint32_t time = 0;
	while (slotIt != slotList.end()) {
		if (slotIt->handler == NULL) {
			error << "FixedSlotSequene::initialize: ObjectId does not exist!"
					<< std::endl;
			count++;
		} else if (slotIt->pollingTimeMs < time) {
			error << "FixedSlotSequence::initialize: Time: "
					<< slotIt->pollingTimeMs
					<< " is smaller than previous with " << time << std::endl;
			count++;
		} else {
			// All ok, print slot.
			// (*slotIt)->print();
		}
		time = slotIt->pollingTimeMs;
		slotIt++;
	}
	if (count > 0) {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

void FixedSlotSequence::addSlot(object_id_t componentId, uint32_t slotTimeMs,
		int8_t executionStep, PeriodicTaskIF* executingTask) {
	this->slotList.insert(FixedSequenceSlot(componentId, slotTimeMs, executionStep,
			executingTask));
	this->current = slotList.begin();
}

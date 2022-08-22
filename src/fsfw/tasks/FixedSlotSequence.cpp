#include "fsfw/tasks/FixedSlotSequence.h"

#include <cstdlib>

#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tasks/FixedTimeslotTaskIF.h"

FixedSlotSequence::FixedSlotSequence(uint32_t setLengthMs) : lengthMs(setLengthMs) {
  current = slotList.begin();
}

FixedSlotSequence::~FixedSlotSequence() {
  // Call the destructor on each list entry.
  slotList.clear();
}

void FixedSlotSequence::executeAndAdvance() {
  current->executableObject->performOperation(current->opcode);
  //	if (returnValue != returnvalue::OK) {
  //		this->sendErrorMessage( returnValue );
  //	}
  // Increment the polling Sequence iterator
  this->current++;
  // Set it to the beginning, if the list's end is reached.
  if (this->current == this->slotList.end()) {
    this->current = this->slotList.begin();
  }
}

uint32_t FixedSlotSequence::getIntervalToNextSlotMs() {
  uint32_t oldTime;
  auto slotListIter = current;
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
  return lengthMs - oldTime + slotListIter->pollingTimeMs;
}

uint32_t FixedSlotSequence::getIntervalToPreviousSlotMs() {
  uint32_t currentTime;
  auto slotListIter = current;
  // Get the pollingTimeMs of the current slot object.
  currentTime = slotListIter->pollingTimeMs;

  // if it is the first slot, calculate difference to last slot
  if (slotListIter == slotList.begin()) {
    return lengthMs - (--slotList.end())->pollingTimeMs + currentTime;
  }
  // get previous slot
  slotListIter--;

  return currentTime - slotListIter->pollingTimeMs;
}

bool FixedSlotSequence::slotFollowsImmediately() {
  uint32_t currentTime = current->pollingTimeMs;
  auto fixedSequenceIter = this->current;
  // Get the pollingTimeMs of the current slot object.
  if (fixedSequenceIter == slotList.begin()) return false;
  fixedSequenceIter--;
  if (fixedSequenceIter->pollingTimeMs == currentTime) {
    return true;
  } else {
    return false;
  }
}

uint32_t FixedSlotSequence::getLengthMs() const { return this->lengthMs; }

void FixedSlotSequence::addSlot(object_id_t componentId, uint32_t slotTimeMs, int8_t executionStep,
                                ExecutableObjectIF* executableObject,
                                PeriodicTaskIF* executingTask) {
  this->slotList.insert(
      FixedSequenceSlot(componentId, slotTimeMs, executionStep, executableObject, executingTask));
  this->current = slotList.begin();
}

ReturnValue_t FixedSlotSequence::checkSequence() const {
  if (slotList.empty()) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "FixedSlotSequence::checkSequence: Slot list is empty!" << std::endl;
#endif
    return FixedTimeslotTaskIF::SLOT_LIST_EMPTY;
  }

  if (customChecker != nullptr) {
    ReturnValue_t result = customChecker(slotList, customCheckArgs);
    if (result != returnvalue::OK) {
      // Continue for now but print error output.
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "FixedSlotSequence::checkSequence:"
                 << " Custom check failed!" << std::endl;
#endif
    }
  }

  uint32_t errorCount = 0;
  uint32_t time = 0;
  for (const auto& slot : slotList) {
    if (slot.executableObject == nullptr) {
      errorCount++;
    } else if (slot.pollingTimeMs < time) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
      sif::error << "FixedSlotSequence::checkSequence: Time: " << slot.pollingTimeMs
                 << " is smaller than previous with " << time << std::endl;
#endif
      errorCount++;
    } else {
      // All ok, print slot.
#if FSFW_CPP_OSTREAM_ENABLED == 1
      // sif::info << "Current slot polling time: " << std::endl;
      // sif::info << std::dec << slotIt->pollingTimeMs << std::endl;
#endif
    }
    time = slot.pollingTimeMs;
  }
#if FSFW_CPP_OSTREAM_ENABLED == 1
  // sif::info << "Number of elements in slot list: "
  //	   << slotList.size() << std::endl;
#endif
  if (errorCount > 0) {
    return returnvalue::FAILED;
  }
  return returnvalue::OK;
}

ReturnValue_t FixedSlotSequence::intializeSequenceAfterTaskCreation() const {
  std::set<ExecutableObjectIF*> uniqueObjects;
  uint32_t count = 0;
  for (const auto& slot : slotList) {
    // Ensure that each unique object is initialized once.
    if (uniqueObjects.find(slot.executableObject) == uniqueObjects.end()) {
      ReturnValue_t result = slot.executableObject->initializeAfterTaskCreation();
      if (result != returnvalue::OK) {
        count++;
      }
      uniqueObjects.emplace(slot.executableObject);
    }
  }
  if (count > 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "FixedSlotSequence::intializeSequenceAfterTaskCreation:"
                  "Counted "
               << count << " failed initializations!" << std::endl;
#endif
    return returnvalue::FAILED;
  }
  return returnvalue::OK;
}

void FixedSlotSequence::addCustomCheck(CustomCheckFunc customChecker_, void* checkerArgs_) {
  customChecker = customChecker_;
  customCheckArgs = checkerArgs_;
}

bool FixedSlotSequence::isEmpty() const { return slotList.empty(); }

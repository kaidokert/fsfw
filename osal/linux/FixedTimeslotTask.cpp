#include "FixedTimeslotTask.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"

#include <limits.h>

uint32_t FixedTimeslotTask::deadlineMissedCount = 0;
const size_t PeriodicTaskIF::MINIMUM_STACK_SIZE = PTHREAD_STACK_MIN;

FixedTimeslotTask::FixedTimeslotTask(const char* name_, int priority_,
		size_t stackSize_, uint32_t periodMs_):
		PosixThread(name_,priority_,stackSize_),pst(periodMs_),started(false) {
}

FixedTimeslotTask::~FixedTimeslotTask() {

}

void* FixedTimeslotTask::taskEntryPoint(void* arg) {
	//The argument is re-interpreted as PollingTask.
	FixedTimeslotTask *originalTask(reinterpret_cast<FixedTimeslotTask*>(arg));
	//The task's functionality is called.
	originalTask->taskFunctionality();
	return nullptr;
}

ReturnValue_t FixedTimeslotTask::startTask() {
	started = true;
	createTask(&taskEntryPoint,this);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t FixedTimeslotTask::sleepFor(uint32_t ms) {
	return PosixThread::sleep((uint64_t)ms*1000000);
}

uint32_t FixedTimeslotTask::getPeriodMs() const {
	return pst.getLengthMs();
}

ReturnValue_t FixedTimeslotTask::addSlot(object_id_t componentId,
		uint32_t slotTimeMs, int8_t executionStep) {
	ExecutableObjectIF* executableObject =
			objectManager->get<ExecutableObjectIF>(componentId);
	if (executableObject != nullptr) {
		pst.addSlot(componentId, slotTimeMs, executionStep,
				executableObject,this);
		return HasReturnvaluesIF::RETURN_OK;
	}

	sif::error << "Component " << std::hex << componentId <<
			" not found, not adding it to pst" << std::dec << std::endl;
	return HasReturnvaluesIF::RETURN_FAILED;
}

ReturnValue_t FixedTimeslotTask::checkSequence() const {
	return pst.checkSequence();
}

void FixedTimeslotTask::taskFunctionality() {
	//Like FreeRTOS pthreads are running as soon as they are created
	if (!started) {
		suspend();
	}

	pst.intializeSequenceAfterTaskCreation();

	//The start time for the first entry is read.
	uint64_t lastWakeTime = getCurrentMonotonicTimeMs();
	uint64_t interval = pst.getIntervalToNextSlotMs();


	//The task's "infinite" inner loop is entered.
	while (1) {
		if (pst.slotFollowsImmediately()) {
			//Do nothing
		} else {
			//The interval for the next polling slot is selected.
			interval = this->pst.getIntervalToPreviousSlotMs();
			//The period is checked and restarted with the new interval.
			//If the deadline was missed, the deadlineMissedFunc is called.
			if(!PosixThread::delayUntil(&lastWakeTime,interval)) {
				//No time left on timer -> we missed the deadline
				missedDeadlineCounter();
			}
		}
		//The device handler for this slot is executed and the next one is chosen.
		this->pst.executeAndAdvance();
	}
}

void FixedTimeslotTask::missedDeadlineCounter() {
	FixedTimeslotTask::deadlineMissedCount++;
	if (FixedTimeslotTask::deadlineMissedCount % 10 == 0) {
		sif::error << "PST missed " << FixedTimeslotTask::deadlineMissedCount
				   << " deadlines." << std::endl;
	}
}

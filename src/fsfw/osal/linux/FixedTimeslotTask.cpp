#include "fsfw/osal/linux/FixedTimeslotTask.h"

#include <climits>

#include "fsfw/serviceinterface/ServiceInterface.h"

const size_t PeriodicTaskIF::MINIMUM_STACK_SIZE = PTHREAD_STACK_MIN;

FixedTimeslotTask::FixedTimeslotTask(const char* name_, TaskPriority priority_, size_t stackSize_,
                                     TaskPeriod periodSeconds_, TaskDeadlineMissedFunction dlmFunc_)
    : FixedTimeslotTaskBase(periodSeconds_, dlmFunc_),
      posixThread(name_, priority_, stackSize_),
      started(false) {}

void* FixedTimeslotTask::taskEntryPoint(void* arg) {
  // The argument is re-interpreted as PollingTask.
  auto* originalTask(reinterpret_cast<FixedTimeslotTask*>(arg));
  // The task's functionality is called.
  originalTask->taskFunctionality();
  return nullptr;
}

ReturnValue_t FixedTimeslotTask::startTask() {
  started = true;
  posixThread.createTask(&taskEntryPoint, this);
  return returnvalue::OK;
}

ReturnValue_t FixedTimeslotTask::sleepFor(uint32_t ms) {
  return PosixThread::sleep((uint64_t)ms * 1000000);
}

[[noreturn]] void FixedTimeslotTask::taskFunctionality() {
  // Like FreeRTOS pthreads are running as soon as they are created
  if (!started) {
    posixThread.suspend();
  }

  // Returnvalue ignored for now
  static_cast<void>(pollingSeqTable.intializeSequenceAfterTaskCreation());

  // The start time for the first entry is read.
  uint64_t lastWakeTime = PosixThread::getCurrentMonotonicTimeMs();
  uint32_t interval = 0;

  // The task's "infinite" inner loop is entered.
  while (true) {
    if (pollingSeqTable.slotFollowsImmediately()) {
      // Do nothing
    } else {
      // The interval for the next polling slot is selected.
      interval = pollingSeqTable.getIntervalToPreviousSlotMs();
      // The period is checked and restarted with the new interval.
      // If the deadline was missed, the deadlineMissedFunc is called.
      if (!PosixThread::delayUntil(&lastWakeTime, interval)) {
        // No time left on timer -> we missed the deadline
        if (dlmFunc != nullptr) {
          dlmFunc();
        }
      }
    }
    // The device handler for this slot is executed and the next one is chosen.
    pollingSeqTable.executeAndAdvance();
  }
}

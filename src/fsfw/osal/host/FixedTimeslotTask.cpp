#include "fsfw/osal/host/FixedTimeslotTask.h"

#include <chrono>
#include <thread>

#include "fsfw/ipc/MutexFactory.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/osal/host/FixedTimeslotTask.h"
#include "fsfw/osal/host/Mutex.h"
#include "fsfw/osal/host/taskHelpers.h"
#include "fsfw/platform.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tasks/ExecutableObjectIF.h"

#if defined(PLATFORM_WIN)
#include <windows.h>

#include "fsfw/osal/windows/winTaskHelpers.h"
#elif defined(PLATFORM_UNIX)
#include <pthread.h>
#endif

FixedTimeslotTask::FixedTimeslotTask(const char* name, TaskPriority setPriority,
                                     TaskStackSize setStack, TaskPeriod setPeriod,
                                     void (*setDeadlineMissedFunc)())
    : started(false),
      pollingSeqTable(setPeriod * 1000),
      taskName(name),
      period(setPeriod),
      deadlineMissedFunc(setDeadlineMissedFunc) {
  // It is propably possible to set task priorities by using the native
  // task handles for Windows / Linux
  mainThread = std::thread(&FixedTimeslotTask::taskEntryPoint, this, this);
#if defined(_WIN32)
  tasks::setTaskPriority(reinterpret_cast<HANDLE>(mainThread.native_handle()), setPriority);
#elif defined(__unix__)
  // TODO: We could reuse existing code here.
#endif
  tasks::insertTaskName(mainThread.get_id(), taskName);
}

FixedTimeslotTask::~FixedTimeslotTask(void) {
  // Do not delete objects, we were responsible for ptrs only.
  terminateThread = true;
  if (mainThread.joinable()) {
    mainThread.join();
  }
}

void FixedTimeslotTask::taskEntryPoint(void* argument) {
  FixedTimeslotTask* originalTask(reinterpret_cast<FixedTimeslotTask*>(argument));

  if (not originalTask->started) {
    // we have to suspend/block here until the task is started.
    // if semaphores are implemented, use them here.
    std::unique_lock<std::mutex> lock(initMutex);
    initCondition.wait(lock);
  }

  this->taskFunctionality();
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::debug << "FixedTimeslotTask::taskEntryPoint: "
                "Returned from taskFunctionality."
             << std::endl;
#endif
}

ReturnValue_t FixedTimeslotTask::startTask() {
  started = true;

  // Notify task to start.
  std::lock_guard<std::mutex> lock(initMutex);
  initCondition.notify_one();

  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t FixedTimeslotTask::sleepFor(uint32_t ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  return HasReturnvaluesIF::RETURN_OK;
}

void FixedTimeslotTask::taskFunctionality() {
  pollingSeqTable.intializeSequenceAfterTaskCreation();

  // A local iterator for the Polling Sequence Table is created to
  // find the start time for the first entry.
  auto slotListIter = pollingSeqTable.current;

  // Get start time for first entry.
  chron_ms interval(slotListIter->pollingTimeMs);
  auto currentStartTime{
      std::chrono::duration_cast<chron_ms>(std::chrono::system_clock::now().time_since_epoch())};
  if (interval.count() > 0) {
    delayForInterval(&currentStartTime, interval);
  }

  /* Enter the loop that defines the task behavior. */
  for (;;) {
    if (terminateThread.load()) {
      break;
    }
    // The component for this slot is executed and the next one is chosen.
    this->pollingSeqTable.executeAndAdvance();
    if (not pollingSeqTable.slotFollowsImmediately()) {
      // we need to wait before executing the current slot
      // this gives us the time to wait:
      interval = chron_ms(this->pollingSeqTable.getIntervalToPreviousSlotMs());
      delayForInterval(&currentStartTime, interval);
      // TODO deadline missed check
    }
  }
}

ReturnValue_t FixedTimeslotTask::addSlot(object_id_t componentId, uint32_t slotTimeMs,
                                         int8_t executionStep) {
  ExecutableObjectIF* executableObject =
      ObjectManager::instance()->get<ExecutableObjectIF>(componentId);
  if (executableObject != nullptr) {
    pollingSeqTable.addSlot(componentId, slotTimeMs, executionStep, executableObject, this);
    return HasReturnvaluesIF::RETURN_OK;
  }

#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::error << "Component " << std::hex << "0x" << componentId
             << "not found, "
                "not adding it to PST.."
             << std::dec << std::endl;
#else
  sif::printError("Component 0x%08x not found, not adding it to PST..\n",
                  static_cast<unsigned int>(componentId));
#endif
  return HasReturnvaluesIF::RETURN_FAILED;
}

ReturnValue_t FixedTimeslotTask::checkSequence() const { return pollingSeqTable.checkSequence(); }

uint32_t FixedTimeslotTask::getPeriodMs() const { return period * 1000; }

bool FixedTimeslotTask::delayForInterval(chron_ms* previousWakeTimeMs, const chron_ms interval) {
  bool shouldDelay = false;
  // Get current wakeup time
  auto currentStartTime =
      std::chrono::duration_cast<chron_ms>(std::chrono::system_clock::now().time_since_epoch());
  /* Generate the tick time at which the task wants to wake. */
  auto nextTimeToWake_ms = (*previousWakeTimeMs) + interval;

  if (currentStartTime < *previousWakeTimeMs) {
    /* The tick count has overflowed since this function was
     lasted called.  In this case the only time we should ever
     actually delay is if the wake time has also    overflowed,
     and the wake time is greater than the tick time.  When this
     is the case it is as if neither time had overflowed. */
    if ((nextTimeToWake_ms < *previousWakeTimeMs) && (nextTimeToWake_ms > currentStartTime)) {
      shouldDelay = true;
    }
  } else {
    /* The tick time has not overflowed.  In this case we will
     delay if either the wake time has overflowed, and/or the
     tick time is less than the wake time. */
    if ((nextTimeToWake_ms < *previousWakeTimeMs) || (nextTimeToWake_ms > currentStartTime)) {
      shouldDelay = true;
    }
  }

  /* Update the wake time ready for the next call. */

  (*previousWakeTimeMs) = nextTimeToWake_ms;

  if (shouldDelay) {
    auto sleepTime = std::chrono::duration_cast<chron_ms>(nextTimeToWake_ms - currentStartTime);
    std::this_thread::sleep_for(sleepTime);
    return true;
  }
  // We are shifting the time in case the deadline was missed like rtems
  (*previousWakeTimeMs) = currentStartTime;
  return false;
}

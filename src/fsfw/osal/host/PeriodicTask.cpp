#include "fsfw/osal/host/PeriodicTask.h"

#include <chrono>
#include <thread>

#include "fsfw/osal/host/Mutex.h"
#include "fsfw/osal/host/taskHelpers.h"
#include "fsfw/platform.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

#if defined(PLATFORM_WIN)
#include <processthreadsapi.h>

#include "fsfw/osal/windows/winTaskHelpers.h"
#elif defined(PLATFORM_UNIX)
#include <pthread.h>
#endif

PeriodicTask::PeriodicTask(const char* name, TaskPriority setPriority, TaskStackSize setStack,
                           TaskPeriod setPeriod, TaskDeadlineMissedFunction dlmFunc_)
    : PeriodicTaskBase(setPeriod, dlmFunc_), started(false), taskName(name) {
  // It is probably possible to set task priorities by using the native
  // task handles for Windows / Linux
  mainThread = std::thread(&PeriodicTask::taskEntryPoint, this, this);
#if defined(PLATFORM_WIN)
  tasks::setTaskPriority(reinterpret_cast<HANDLE>(mainThread.native_handle()), setPriority);
#elif defined(PLATFORM_UNIX)
  // TODO: We could reuse existing code here.
#endif
  tasks::insertTaskName(mainThread.get_id(), taskName);
}

PeriodicTask::~PeriodicTask() {
  // Do not delete objects, we were responsible for ptrs only.
  terminateThread = true;
  if (mainThread.joinable()) {
    mainThread.join();
  }
}

void PeriodicTask::taskEntryPoint(void* argument) {
  auto* originalTask(reinterpret_cast<PeriodicTask*>(argument));

  if (not originalTask->started) {
    // we have to suspend/block here until the task is started.
    // if semaphores are implemented, use them here.
    std::unique_lock<std::mutex> lock(initMutex);
    initCondition.wait(lock);
  }

  this->taskFunctionality();
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::debug << "PeriodicTask::taskEntryPoint: "
                "Returned from taskFunctionality."
             << std::endl;
#endif
}

ReturnValue_t PeriodicTask::startTask() {
  started = true;

  // Notify task to start.
  std::lock_guard<std::mutex> lock(initMutex);
  initCondition.notify_one();

  return returnvalue::OK;
}

ReturnValue_t PeriodicTask::sleepFor(uint32_t ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  return returnvalue::OK;
}

void PeriodicTask::taskFunctionality() {
  initObjsAfterTaskCreation();

  std::chrono::milliseconds periodChrono(static_cast<uint32_t>(period * 1000));
  auto currentStartTime{std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch())};
  /* Enter the loop that defines the task behavior. */
  for (;;) {
    if (terminateThread.load()) {
      break;
    }
    for (const auto& objectPair : objectList) {
      objectPair.first->performOperation(objectPair.second);
    }
    if (not delayForInterval(&currentStartTime, periodChrono)) {
      if (dlmFunc != nullptr) {
        this->dlmFunc();
      }
    }
  }
}

bool PeriodicTask::delayForInterval(chron_ms* previousWakeTimeMs, const chron_ms interval) {
  bool shouldDelay = false;
  // Get current wakeup time
  auto currentStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch());
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
    auto sleepTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(nextTimeToWake_ms - currentStartTime);
    std::this_thread::sleep_for(sleepTime);
    return true;
  }
  // We are shifting the time in case the deadline was missed like rtems
  (*previousWakeTimeMs) = currentStartTime;
  return false;
}

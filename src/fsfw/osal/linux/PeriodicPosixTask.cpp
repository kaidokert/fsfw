#include "PeriodicPosixTask.h"

#include "fsfw/serviceinterface.h"
#include "fsfw/tasks/ExecutableObjectIF.h"

PeriodicPosixTask::PeriodicPosixTask(const char* name_, int priority_, size_t stackSize_,
                                     TaskPeriod period_, TaskDeadlineMissedFunction dlmFunc_)
    : PeriodicTaskBase(period_, dlmFunc_),
      posixThread(name_, priority_, stackSize_),
      started(false) {}

void* PeriodicPosixTask::taskEntryPoint(void* arg) {
  // The argument is re-interpreted as PollingTask.
  auto* originalTask(reinterpret_cast<PeriodicPosixTask*>(arg));
  // The task's functionality is called.
  originalTask->taskFunctionality();
  return nullptr;
}

ReturnValue_t PeriodicPosixTask::sleepFor(uint32_t ms) {
  return PosixThread::sleep(static_cast<uint64_t>(ms) * 1000000);
}

ReturnValue_t PeriodicPosixTask::startTask() {
  if (isEmpty()) {
    return returnvalue::FAILED;
  }
  started = true;
  posixThread.createTask(&taskEntryPoint, this);
  return returnvalue::OK;
}

[[noreturn]] void PeriodicPosixTask::taskFunctionality() {
  if (not started) {
    posixThread.suspend();
  }

  initObjsAfterTaskCreation();

  uint64_t lastWakeTime = PosixThread::getCurrentMonotonicTimeMs();
  uint64_t periodMs = getPeriodMs();
  // The task's "infinite" inner loop is entered.
  while (true) {
    for (auto const& objOpCodePair : objectList) {
      objOpCodePair.first->performOperation(objOpCodePair.second);
    }

    if (not PosixThread::delayUntil(&lastWakeTime, periodMs)) {
      if (dlmFunc != nullptr) {
        dlmFunc();
      }
    }
  }
}

#include "fsfw/osal/linux/PeriodicPosixTask.h"

#include <set>
#include <cerrno>

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tasks/ExecutableObjectIF.h"

PeriodicPosixTask::PeriodicPosixTask(const char* name_, int priority_, size_t stackSize_,
                                     uint32_t period_, TaskDeadlineMissedFunction dlMissedFunc_)
    : PosixThread(name_, priority_, stackSize_),
      PeriodicTaskBase(period_, dlMissedFunc_),
      started(false) {}

PeriodicPosixTask::~PeriodicPosixTask() {
  // Not Implemented
}

void* PeriodicPosixTask::taskEntryPoint(void* arg) {
  // The argument is re-interpreted as PollingTask.
  PeriodicPosixTask* originalTask(reinterpret_cast<PeriodicPosixTask*>(arg));
  // The task's functionality is called.
  originalTask->taskFunctionality();
  return nullptr;
}

ReturnValue_t PeriodicPosixTask::sleepFor(uint32_t ms) {
  return PosixThread::sleep(static_cast<uint64_t>(ms * 1000000));
}

ReturnValue_t PeriodicPosixTask::startTask(void) {
  if (isEmpty()) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  started = true;
  PosixThread::createTask(&taskEntryPoint, this);
  return HasReturnvaluesIF::RETURN_OK;
}

void PeriodicPosixTask::taskFunctionality(void) {
  if (not started) {
    suspend();
  }

  initObjsAfterTaskCreation();

  uint64_t lastWakeTime = getCurrentMonotonicTimeMs();
  // The task's "infinite" inner loop is entered.
  while (1) {
    for (auto const& objOpCodePair : objectList) {
      objOpCodePair.first->performOperation(objOpCodePair.second);
    }

    if (not PosixThread::delayUntil(&lastWakeTime, periodMs)) {
      if (this->deadlineMissedFunc != nullptr) {
        this->deadlineMissedFunc();
      }
    }
  }
}

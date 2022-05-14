#include "fsfw/osal/linux/PeriodicPosixTask.h"

#include <errno.h>

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tasks/ExecutableObjectIF.h"

PeriodicPosixTask::PeriodicPosixTask(const char* name_, int priority_, size_t stackSize_,
                                     uint32_t period_, void(deadlineMissedFunc_)())
    : PosixThread(name_, priority_, stackSize_),
      objectList(),
      started(false),
      periodMs(period_),
      deadlineMissedFunc(deadlineMissedFunc_) {}

PeriodicPosixTask::~PeriodicPosixTask() {
  // Not Implemented
}

void* PeriodicPosixTask::taskEntryPoint(void* arg) {
  // The argument is re-interpreted as PollingTask.
  PeriodicPosixTask* originalTask(reinterpret_cast<PeriodicPosixTask*>(arg));
  // The task's functionality is called.
  originalTask->taskFunctionality();
  return NULL;
}

ReturnValue_t PeriodicPosixTask::addComponent(object_id_t object, uint8_t opCode) {
  ExecutableObjectIF* newObject = ObjectManager::instance()->get<ExecutableObjectIF>(object);
  return addComponent(newObject, opCode);
}

ReturnValue_t PeriodicPosixTask::addComponent(ExecutableObjectIF* object, uint8_t opCode) {
  if (object == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "PeriodicTask::addComponent: Invalid object. Make sure"
               << " it implements ExecutableObjectIF!" << std::endl;
#else
    sif::printError(
        "PeriodicTask::addComponent: Invalid object. Make sure it "
        "implements ExecutableObjectIF!\n");
#endif
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  objectList.emplace(object, opCode);
  object->setTaskIF(this);

  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t PeriodicPosixTask::sleepFor(uint32_t ms) {
  return PosixThread::sleep((uint64_t)ms * 1000000);
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

uint32_t PeriodicPosixTask::getPeriodMs() const { return periodMs; }

bool PeriodicPosixTask::isEmpty() const { return objectList.empty(); }

ReturnValue_t PeriodicPosixTask::initObjsAfterTaskCreation() {
  std::multiset<ExecutableObjectIF*> uniqueObjects;
  ReturnValue_t status = HasReturnvaluesIF::RETURN_OK;
  uint32_t count = 0;
  for (const auto& obj : objectList) {
    // Ensure that each unique object is initialized once.
    if (uniqueObjects.find(obj.first) == uniqueObjects.end()) {
      ReturnValue_t result = obj.first->initializeAfterTaskCreation();
      if (result != HasReturnvaluesIF::RETURN_OK) {
        count++;
        status = result;
      }
      uniqueObjects.emplace(obj.first);
    }
  }
  if (count > 0) {
  }
  return status;
}

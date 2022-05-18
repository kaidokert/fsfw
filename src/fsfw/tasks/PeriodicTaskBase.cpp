#include <fsfw/objectmanager/ObjectManager.h>
#include "PeriodicTaskBase.h"

#include <set>

PeriodicTaskBase::PeriodicTaskBase(uint32_t periodMs_,
    TaskDeadlineMissedFunction deadlineMissedFunc_)
  : periodMs(periodMs_), deadlineMissedFunc(deadlineMissedFunc_) {}

uint32_t PeriodicTaskBase::getPeriodMs() const { return periodMs; }

bool PeriodicTaskBase::isEmpty() const override {
  return objectList.empty();
}

ReturnValue_t PeriodicTaskBase::initObjsAfterTaskCreation() {
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

ReturnValue_t PeriodicTaskBase::addComponent(object_id_t object, uint8_t opCode) {
  ExecutableObjectIF* newObject = ObjectManager::instance()->get<ExecutableObjectIF>(object);
  return addComponent(newObject, opCode);
}

ReturnValue_t PeriodicTaskBase::addComponent(ExecutableObjectIF* object, uint8_t opCode) {
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
  objectList.push_back({object, opCode});
  object->setTaskIF(this);

  return HasReturnvaluesIF::RETURN_OK;
}

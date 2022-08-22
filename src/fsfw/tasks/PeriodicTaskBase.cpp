#include "PeriodicTaskBase.h"

#include <set>

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface.h"

PeriodicTaskBase::PeriodicTaskBase(TaskPeriod period_, TaskDeadlineMissedFunction dlmFunc_)
    : period(period_), dlmFunc(dlmFunc_) {
  // Hints at configuration error
  if (PeriodicTaskBase::getPeriodMs() <= 1) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "Passed task period 0 or smaller than 1 ms" << std::endl;
#else
    sif::printWarning("Passed task period 0 or smaller than 1ms\n");
#endif
  }
}

uint32_t PeriodicTaskBase::getPeriodMs() const { return static_cast<uint32_t>(period * 1000); }

bool PeriodicTaskBase::isEmpty() const { return objectList.empty(); }

ReturnValue_t PeriodicTaskBase::addComponent(object_id_t object) { return addComponent(object, 0); }

ReturnValue_t PeriodicTaskBase::addComponent(ExecutableObjectIF* object) {
  return addComponent(object, 0);
}

ReturnValue_t PeriodicTaskBase::initObjsAfterTaskCreation() {
  std::set<ExecutableObjectIF*> uniqueObjects;
  ReturnValue_t status = returnvalue::OK;
  uint32_t count = 0;
  for (const auto& obj : objectList) {
    // Ensure that each unique object is initialized once.
    if (uniqueObjects.find(obj.first) == uniqueObjects.end()) {
      ReturnValue_t result = obj.first->initializeAfterTaskCreation();
      if (result != returnvalue::OK) {
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
  auto* newObject = ObjectManager::instance()->get<ExecutableObjectIF>(object);
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
    return returnvalue::FAILED;
  }
  objectList.push_back({object, opCode});
  object->setTaskIF(this);

  return returnvalue::OK;
}

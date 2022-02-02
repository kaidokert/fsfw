#include "fsfw/osal/rtems/PeriodicTask.h"

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tasks/ExecutableObjectIF.h"

PeriodicTask::PeriodicTask(const char* name, rtems_task_priority setPriority, size_t setStack,
                           rtems_interval setPeriod, void (*setDeadlineMissedFunc)())
    : RTEMSTaskBase(setPriority, setStack, name),
      periodTicks(RtemsBasic::convertMsToTicks(setPeriod)),
      deadlineMissedFunc(setDeadlineMissedFunc) {}

PeriodicTask::~PeriodicTask(void) {
  /* Do not delete objects, we were responsible for pointers only. */
  rtems_rate_monotonic_delete(periodId);
}

rtems_task PeriodicTask::taskEntryPoint(rtems_task_argument argument) {
  /* The argument is re-interpreted as MultiObjectTask. The Task object is global,
  so it is found from any place. */
  PeriodicTask* originalTask(reinterpret_cast<PeriodicTask*>(argument));
  return originalTask->taskFunctionality();
  ;
}

ReturnValue_t PeriodicTask::startTask() {
  rtems_status_code status =
      rtems_task_start(id, PeriodicTask::taskEntryPoint, rtems_task_argument((void*)this));
  if (status != RTEMS_SUCCESSFUL) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "ObjectTask::startTask for " << std::hex << this->getId() << std::dec
               << " failed." << std::endl;
#endif
  }
  switch (status) {
    case RTEMS_SUCCESSFUL:
      /* Task started successfully */
      return HasReturnvaluesIF::RETURN_OK;
    default:
      /* RTEMS_INVALID_ADDRESS - invalid task entry point
              RTEMS_INVALID_ID - invalid task id
              RTEMS_INCORRECT_STATE - task not in the dormant state
              RTEMS_ILLEGAL_ON_REMOTE_OBJECT - cannot start remote task */
      return HasReturnvaluesIF::RETURN_FAILED;
  }
}

ReturnValue_t PeriodicTask::sleepFor(uint32_t ms) { return RTEMSTaskBase::sleepFor(ms); }

void PeriodicTask::taskFunctionality() {
  RTEMSTaskBase::setAndStartPeriod(periodTicks, &periodId);
  for (const auto& object : objectList) {
    object->initializeAfterTaskCreation();
  }
  /* The task's "infinite" inner loop is entered. */
  while (1) {
    for (const auto& object : objectList) {
      object->performOperation();
    }
    rtems_status_code status = RTEMSTaskBase::restartPeriod(periodTicks, periodId);
    if (status == RTEMS_TIMEOUT) {
      if (this->deadlineMissedFunc != nullptr) {
        this->deadlineMissedFunc();
      }
    }
  }
}

ReturnValue_t PeriodicTask::addComponent(object_id_t object) {
  ExecutableObjectIF* newObject = ObjectManager::instance()->get<ExecutableObjectIF>(object);
  if (newObject == nullptr) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  objectList.push_back(newObject);
  newObject->setTaskIF(this);

  return HasReturnvaluesIF::RETURN_OK;
}

uint32_t PeriodicTask::getPeriodMs() const { return RtemsBasic::convertTicksToMs(periodTicks); }

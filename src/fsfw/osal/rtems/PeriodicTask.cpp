#include "fsfw/osal/rtems/PeriodicTask.h"

#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "fsfw/tasks/ExecutableObjectIF.h"

PeriodicTask::PeriodicTask(const char* name, rtems_task_priority setPriority, size_t setStack,
                           TaskPeriod setPeriod, TaskDeadlineMissedFunction dlmFunc_)
    : PeriodicTaskBase(setPeriod, dlmFunc_),
      RTEMSTaskBase(setPriority, setStack, name),
      periodTicks(RtemsBasic::convertMsToTicks(static_cast<uint32_t>(setPeriod * 1000.0))) {}

PeriodicTask::~PeriodicTask() {
  /* Do not delete objects, we were responsible for pointers only. */
  rtems_rate_monotonic_delete(periodId);
}

rtems_task PeriodicTask::taskEntryPoint(rtems_task_argument argument) {
  /* The argument is re-interpreted as MultiObjectTask. The Task object is global,
  so it is found from any place. */
  auto* originalTask(reinterpret_cast<PeriodicTask*>(argument));
  return originalTask->taskFunctionality();
  ;
}

ReturnValue_t PeriodicTask::startTask() {
  rtems_status_code status =
      rtems_task_start(id, PeriodicTask::taskEntryPoint, rtems_task_argument((void*)this));
  if (status != RTEMS_SUCCESSFUL) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "PeriodicTask::startTask for " << std::hex << this->getId() << std::dec
               << " failed" << std::endl;
#else
    sif::printError("PeriodicTask::startTask for 0x%08x failed\n", getId());
#endif
  }
  switch (status) {
    case RTEMS_SUCCESSFUL:
      /* Task started successfully */
      return returnvalue::OK;
    default:
      /* RTEMS_INVALID_ADDRESS - invalid task entry point
              RTEMS_INVALID_ID - invalid task id
              RTEMS_INCORRECT_STATE - task not in the dormant state
              RTEMS_ILLEGAL_ON_REMOTE_OBJECT - cannot start remote task */
      return returnvalue::FAILED;
  }
}

ReturnValue_t PeriodicTask::sleepFor(uint32_t ms) { return RTEMSTaskBase::sleepFor(ms); }

[[noreturn]] void PeriodicTask::taskFunctionality() {
  RTEMSTaskBase::setAndStartPeriod(periodTicks, &periodId);
  initObjsAfterTaskCreation();

  /* The task's "infinite" inner loop is entered. */
  while (true) {
    for (const auto& objectPair : objectList) {
      objectPair.first->performOperation(objectPair.second);
    }
    rtems_status_code status = RTEMSTaskBase::restartPeriod(periodTicks, periodId);
    if (status == RTEMS_TIMEOUT) {
      if (dlmFunc != nullptr) {
        dlmFunc();
      }
    }
  }
}

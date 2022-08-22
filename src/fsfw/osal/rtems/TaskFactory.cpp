#include "fsfw/tasks/TaskFactory.h"

#include "fsfw/osal/rtems/FixedTimeslotTask.h"
#include "fsfw/osal/rtems/PeriodicTask.h"
#include "fsfw/osal/rtems/RtemsBasic.h"
#include "fsfw/returnvalues/returnvalue.h"

// TODO: Different variant than the lazy loading in QueueFactory. What's better and why?
TaskFactory* TaskFactory::factoryInstance = new TaskFactory();

TaskFactory::TaskFactory() = default;

TaskFactory::~TaskFactory() = default;

TaskFactory* TaskFactory::instance() { return TaskFactory::factoryInstance; }

PeriodicTaskIF* TaskFactory::createPeriodicTask(
    TaskName name_, TaskPriority taskPriority_, TaskStackSize stackSize_,
    TaskPeriod periodInSeconds_, TaskDeadlineMissedFunction deadLineMissedFunction_) {
  return static_cast<PeriodicTaskIF*>(new PeriodicTask(name_, taskPriority_, stackSize_,
                                                       periodInSeconds_, deadLineMissedFunction_));
}

FixedTimeslotTaskIF* TaskFactory::createFixedTimeslotTask(
    TaskName name_, TaskPriority taskPriority_, TaskStackSize stackSize_,
    TaskPeriod periodInSeconds_, TaskDeadlineMissedFunction deadLineMissedFunction_) {
  return static_cast<FixedTimeslotTaskIF*>(new FixedTimeslotTask(
      name_, taskPriority_, stackSize_, periodInSeconds_, deadLineMissedFunction_));
}

ReturnValue_t TaskFactory::deleteTask(PeriodicTaskIF* task) {
  // This should call the OS specific destructor
  delete (dynamic_cast<PeriodicTask*>(task));
  return returnvalue::FAILED;
}

ReturnValue_t TaskFactory::delayTask(uint32_t delayMs) {
  rtems_task_wake_after(RtemsBasic::convertMsToTicks(delayMs));
  // Only return value is "RTEMS_SUCCESSFUL - always successful" so it has been neglected
  return returnvalue::OK;
}

void TaskFactory::printMissedDeadline() {
  /* TODO: Implement */
  return;
}

#include "fsfw/tasks/TaskFactory.h"

#include "fsfw/osal/freertos/FixedTimeslotTask.h"
#include "fsfw/osal/freertos/PeriodicTask.h"
#include "fsfw/returnvalues/returnvalue.h"

TaskFactory* TaskFactory::factoryInstance = new TaskFactory();

TaskFactory::~TaskFactory() {}

TaskFactory* TaskFactory::instance() { return TaskFactory::factoryInstance; }

PeriodicTaskIF* TaskFactory::createPeriodicTask(
    TaskName name_, TaskPriority taskPriority_, TaskStackSize stackSize_, TaskPeriod period_,
    TaskDeadlineMissedFunction deadLineMissedFunction_) {
  return dynamic_cast<PeriodicTaskIF*>(
      new PeriodicTask(name_, taskPriority_, stackSize_, period_, deadLineMissedFunction_));
}

/**
 * Keep in Mind that you need to call before this vTaskStartScheduler()!
 */
FixedTimeslotTaskIF* TaskFactory::createFixedTimeslotTask(
    TaskName name_, TaskPriority taskPriority_, TaskStackSize stackSize_, TaskPeriod period_,
    TaskDeadlineMissedFunction deadLineMissedFunction_) {
  return dynamic_cast<FixedTimeslotTaskIF*>(
      new FixedTimeslotTask(name_, taskPriority_, stackSize_, period_, deadLineMissedFunction_));
}

ReturnValue_t TaskFactory::deleteTask(PeriodicTaskIF* task) {
  if (task == nullptr) {
    // delete self
    vTaskDelete(nullptr);
    return returnvalue::OK;
  } else {
    // TODO not implemented
    return returnvalue::FAILED;
  }
}

ReturnValue_t TaskFactory::delayTask(uint32_t delayMs) {
  vTaskDelay(pdMS_TO_TICKS(delayMs));
  return returnvalue::OK;
}

void TaskFactory::printMissedDeadline() {
  /* TODO: Implement */
  return;
}

TaskFactory::TaskFactory() {}

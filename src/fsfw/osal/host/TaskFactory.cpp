#include "fsfw/tasks/TaskFactory.h"

#include <chrono>

#include "fsfw/osal/host/FixedTimeslotTask.h"
#include "fsfw/osal/host/PeriodicTask.h"
#include "fsfw/osal/host/taskHelpers.h"
#include "fsfw/returnvalues/HasReturnvaluesIF.h"
#include "fsfw/serviceinterface.h"
#include "fsfw/tasks/PeriodicTaskIF.h"

TaskFactory* TaskFactory::factoryInstance = new TaskFactory();

// Not used for the host implementation for now because C++ thread abstraction is used
const size_t PeriodicTaskIF::MINIMUM_STACK_SIZE = 0;

TaskFactory::TaskFactory() = default;

TaskFactory::~TaskFactory() = default;

TaskFactory* TaskFactory::instance() { return TaskFactory::factoryInstance; }

PeriodicTaskIF* TaskFactory::createPeriodicTask(
    TaskName name_, TaskPriority taskPriority_, TaskStackSize stackSize_,
    TaskPeriod periodInSeconds_, TaskDeadlineMissedFunction deadLineMissedFunction_) {
  return new PeriodicTask(name_, taskPriority_, stackSize_, periodInSeconds_,
                          deadLineMissedFunction_);
}

FixedTimeslotTaskIF* TaskFactory::createFixedTimeslotTask(
    TaskName name_, TaskPriority taskPriority_, TaskStackSize stackSize_,
    TaskPeriod periodInSeconds_, TaskDeadlineMissedFunction deadLineMissedFunction_) {
  return new FixedTimeslotTask(name_, taskPriority_, stackSize_, periodInSeconds_,
                               deadLineMissedFunction_);
}

ReturnValue_t TaskFactory::deleteTask(PeriodicTaskIF* task) {
  // This might block for some time!
  delete task;
  return HasReturnvaluesIF::RETURN_FAILED;
}

ReturnValue_t TaskFactory::delayTask(uint32_t delayMs) {
  std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
  return HasReturnvaluesIF::RETURN_OK;
}

void TaskFactory::printMissedDeadline() {
  std::string name = tasks::getTaskName(std::this_thread::get_id());
  FSFW_LOGW("printMissedDeadline: {}\n", name);
}

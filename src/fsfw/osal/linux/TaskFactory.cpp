#include "fsfw/tasks/TaskFactory.h"

#include "fsfw/osal/linux/FixedTimeslotTask.h"
#include "fsfw/osal/linux/PeriodicPosixTask.h"
#include "fsfw/returnvalues/returnvalue.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

// TODO: Different variant than the lazy loading in QueueFactory. What's better and why?
TaskFactory* TaskFactory::factoryInstance = new TaskFactory();

TaskFactory::~TaskFactory() = default;

TaskFactory* TaskFactory::instance() { return TaskFactory::factoryInstance; }

PeriodicTaskIF* TaskFactory::createPeriodicTask(
    TaskName name_, TaskPriority taskPriority_, TaskStackSize stackSize_,
    TaskPeriod periodInSeconds_, TaskDeadlineMissedFunction deadLineMissedFunction_) {
  return new PeriodicPosixTask(name_, taskPriority_, stackSize_, periodInSeconds_,
                               deadLineMissedFunction_);
}

FixedTimeslotTaskIF* TaskFactory::createFixedTimeslotTask(
    TaskName name_, TaskPriority taskPriority_, TaskStackSize stackSize_,
    TaskPeriod periodInSeconds_, TaskDeadlineMissedFunction deadLineMissedFunction_) {
  return new FixedTimeslotTask(name_, taskPriority_, stackSize_, periodInSeconds_,
                               deadLineMissedFunction_);
}

ReturnValue_t TaskFactory::deleteTask(PeriodicTaskIF* task) {
  // TODO not implemented
  return returnvalue::FAILED;
}

ReturnValue_t TaskFactory::delayTask(uint32_t delayMs) {
  return PosixThread::sleep(delayMs * 1000000ull);
}

void TaskFactory::printMissedDeadline() {
  char name[20] = {0};
  int status = pthread_getname_np(pthread_self(), name, sizeof(name));
#if FSFW_CPP_OSTREAM_ENABLED == 1
  if (status == 0) {
    sif::warning << "task::printMissedDeadline: " << name << "" << std::endl;
  } else {
    sif::warning << "task::printMissedDeadline: Unknown task name" << status << std::endl;
  }
#else
  if (status == 0) {
    sif::printWarning("task::printMissedDeadline: %s\n", name);
  } else {
    sif::printWarning("task::printMissedDeadline: Unknown task name\n", name);
  }
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
}

TaskFactory::TaskFactory() {}

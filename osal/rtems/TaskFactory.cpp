#include <framework/tasks/TaskFactory.h>
#include "MultiObjectTask.h"
#include "PollingTask.h"
#include "InitTask.h"
#include <framework/returnvalues/HasReturnvaluesIF.h>

//TODO: Different variant than the lazy loading in QueueFactory. What's better and why?
TaskFactory* TaskFactory::factoryInstance = new TaskFactory();

TaskFactory::~TaskFactory() {
}

TaskFactory* TaskFactory::instance() {
	return TaskFactory::factoryInstance;
}

PeriodicTaskIF* TaskFactory::createPeriodicTask(OSAL::TaskName name_,OSAL::TaskPriority taskPriority_,OSAL::TaskStackSize stackSize_,OSAL::TaskPeriod periodInSeconds_,OSAL::TaskDeadlineMissedFunction deadLineMissedFunction_) {
	rtems_interval taskPeriod = periodInSeconds_ * Clock::getTicksPerSecond();

	return static_cast<PeriodicTaskIF*>(new MultiObjectTask(name_,taskPriority_,stackSize_,taskPeriod,deadLineMissedFunction_));
}

FixedTimeslotTaskIF* TaskFactory::createFixedTimeslotTask(OSAL::TaskName name_,OSAL::TaskPriority taskPriority_,OSAL::TaskStackSize stackSize_,OSAL::TaskPeriod periodInSeconds_,OSAL::TaskDeadlineMissedFunction deadLineMissedFunction_) {
	rtems_interval taskPeriod = periodInSeconds_ * Clock::getTicksPerSecond();
	return static_cast<FixedTimeslotTaskIF*>(new PollingTask(name_,taskPriority_,stackSize_,taskPeriod,deadLineMissedFunction_));
}

ReturnValue_t TaskFactory::deleteTask(PeriodicTaskIF* task) {
	//TODO not implemented
	return HasReturnvaluesIF::RETURN_FAILED;
}

TaskFactory::TaskFactory() {
}

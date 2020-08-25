#include "FixedTimeslotTask.h"
#include "PeriodicPosixTask.h"
#include "../../tasks/TaskFactory.h"
#include "../../returnvalues/HasReturnvaluesIF.h"

//TODO: Different variant than the lazy loading in QueueFactory. What's better and why?
TaskFactory* TaskFactory::factoryInstance = new TaskFactory();

TaskFactory::~TaskFactory() {
}

TaskFactory* TaskFactory::instance() {
	return TaskFactory::factoryInstance;
}

PeriodicTaskIF* TaskFactory::createPeriodicTask(TaskName name_,
		TaskPriority taskPriority_,TaskStackSize stackSize_,
		TaskPeriod periodInSeconds_,
		TaskDeadlineMissedFunction deadLineMissedFunction_) {
	return new PeriodicPosixTask(name_, taskPriority_,stackSize_,
			periodInSeconds_ * 1000, deadLineMissedFunction_);
}

FixedTimeslotTaskIF* TaskFactory::createFixedTimeslotTask(TaskName name_,
		TaskPriority taskPriority_,TaskStackSize stackSize_,
		TaskPeriod periodInSeconds_,
		TaskDeadlineMissedFunction deadLineMissedFunction_) {
	return new FixedTimeslotTask(name_, taskPriority_,stackSize_,
			periodInSeconds_*1000);
}

ReturnValue_t TaskFactory::deleteTask(PeriodicTaskIF* task) {
	//TODO not implemented
	return HasReturnvaluesIF::RETURN_FAILED;
}

ReturnValue_t TaskFactory::delayTask(uint32_t delayMs){
	return PosixThread::sleep(delayMs*1000000ull);
}

TaskFactory::TaskFactory() {
}

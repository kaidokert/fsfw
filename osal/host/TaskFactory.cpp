#include "taskHelpers.h"
#include "../../tasks/TaskFactory.h"
#include "../../osal/host/FixedTimeslotTask.h"
#include "../../osal/host/PeriodicTask.h"
#include "../../returnvalues/HasReturnvaluesIF.h"
#include "../../tasks/PeriodicTaskIF.h"
#include "../../serviceinterface/ServiceInterface.h"

#include <chrono>

TaskFactory* TaskFactory::factoryInstance = new TaskFactory();

// Not used for the host implementation for now because C++ thread abstraction is used
const size_t PeriodicTaskIF::MINIMUM_STACK_SIZE = 0;

TaskFactory::TaskFactory() {
}

TaskFactory::~TaskFactory() {
}

TaskFactory* TaskFactory::instance() {
	return TaskFactory::factoryInstance;
}

PeriodicTaskIF* TaskFactory::createPeriodicTask(TaskName name_,
		TaskPriority taskPriority_,TaskStackSize stackSize_,
		TaskPeriod periodInSeconds_,
		TaskDeadlineMissedFunction deadLineMissedFunction_) {
	return new PeriodicTask(name_, taskPriority_, stackSize_, periodInSeconds_,
			deadLineMissedFunction_);
}

FixedTimeslotTaskIF* TaskFactory::createFixedTimeslotTask(TaskName name_,
		TaskPriority taskPriority_,TaskStackSize stackSize_,
		TaskPeriod periodInSeconds_,
		TaskDeadlineMissedFunction deadLineMissedFunction_) {
	return new FixedTimeslotTask(name_, taskPriority_, stackSize_,
	        periodInSeconds_, deadLineMissedFunction_);
}

ReturnValue_t TaskFactory::deleteTask(PeriodicTaskIF* task) {
	// This might block for some time!
	delete task;
	return HasReturnvaluesIF::RETURN_FAILED;
}

ReturnValue_t TaskFactory::delayTask(uint32_t delayMs){
	std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
	return HasReturnvaluesIF::RETURN_OK;
}

void TaskFactory::printMissedDeadline() {
    std::string name = tasks::getTaskName(std::this_thread::get_id());
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "TaskFactory::printMissedDeadline: " << name << std::endl;
#else
    sif::printWarning("TaskFactory::printMissedDeadline: %s\n", name);
#endif /* FSFW_CPP_OSTREAM_ENABLED == 1 */
}



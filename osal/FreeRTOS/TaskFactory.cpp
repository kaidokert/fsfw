#include "../../tasks/TaskFactory.h"
#include "../../returnvalues/HasReturnvaluesIF.h"

#include "PeriodicTask.h"
#include "FixedTimeslotTask.h"


TaskFactory* TaskFactory::factoryInstance = new TaskFactory();

TaskFactory::~TaskFactory() {
}

TaskFactory* TaskFactory::instance() {
	return TaskFactory::factoryInstance;
}
/***
 * Keep in Mind that you need to call before this vTaskStartScheduler()!
 * High taskPriority_ number means high priority.
 */
PeriodicTaskIF* TaskFactory::createPeriodicTask(TaskName name_,
		TaskPriority taskPriority_, TaskStackSize stackSize_,
		TaskPeriod period_,
		TaskDeadlineMissedFunction deadLineMissedFunction_) {
	return (PeriodicTaskIF*) (new PeriodicTask(name_, taskPriority_, stackSize_,
			period_, deadLineMissedFunction_));
}
/***
 * Keep in Mind that you need to call before this vTaskStartScheduler()!
 */
FixedTimeslotTaskIF* TaskFactory::createFixedTimeslotTask(TaskName name_,
		TaskPriority taskPriority_, TaskStackSize stackSize_,
		TaskPeriod period_,
		TaskDeadlineMissedFunction deadLineMissedFunction_) {
	return (FixedTimeslotTaskIF*) (new FixedTimeslotTask(name_, taskPriority_,
			stackSize_, period_, deadLineMissedFunction_));
}

ReturnValue_t TaskFactory::deleteTask(PeriodicTaskIF* task) {
	if (task == NULL) {
		//delete self
		vTaskDelete(NULL);
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		//TODO not implemented
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

ReturnValue_t TaskFactory::delayTask(uint32_t delayMs) {
	vTaskDelay(pdMS_TO_TICKS(delayMs));
	return HasReturnvaluesIF::RETURN_OK;
}

TaskFactory::TaskFactory() {
}

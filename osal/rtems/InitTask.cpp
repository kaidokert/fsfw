#include "InitTask.h"
#include "RtemsBasic.h"



InitTask::InitTask() {
}

InitTask::~InitTask() {
}

void InitTask::deleteTask(){
	rtems_task_delete(RTEMS_SELF);
}

ReturnValue_t InitTask::startTask() {
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t InitTask::sleepFor(uint32_t ms) {
	rtems_status_code status = rtems_task_wake_after(RtemsBasic::convertMsToTicks(ms));
	return RtemsBasic::convertReturnCode(status);
}

uint32_t InitTask::getPeriodMs() const {
	return 0;
}

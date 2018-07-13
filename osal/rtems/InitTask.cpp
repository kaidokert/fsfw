#include "InitTask.h"
#include "RtemsBasic.h"



InitTask::InitTask() {
}

InitTask::~InitTask() {
	rtems_task_delete(RTEMS_SELF);
}

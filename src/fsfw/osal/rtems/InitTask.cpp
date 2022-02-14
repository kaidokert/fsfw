#include "fsfw/osal/rtems/InitTask.h"

#include "fsfw/osal/rtems/RtemsBasic.h"

InitTask::InitTask() {}

InitTask::~InitTask() { rtems_task_delete(RTEMS_SELF); }

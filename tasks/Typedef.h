#ifndef FRAMEWORK_TASKS_TYPEDEF_H_
#define FRAMEWORK_TASKS_TYPEDEF_H_


#ifndef API
#error Please specify Operating System API. Supported: API=RTEMS_API
#elif API == RTEMS_API
#include <framework/osal/rtems/RtemsBasic.h>
namespace OSAL{
	typedef const char* TaskName;
	typedef rtems_task_priority	TaskPriority;
	typedef size_t	TaskStackSize;
	typedef double TaskPeriod;
	typedef void (*TaskDeadlineMissedFunction)();
};
#endif




#endif /* FRAMEWORK_TASKS_TYPEDEF_H_ */

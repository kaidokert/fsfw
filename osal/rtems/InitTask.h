#ifndef OS_RTEMS_INITTASK_H_
#define OS_RTEMS_INITTASK_H_

#include <framework/tasks/PeriodicTaskIF.h>

//TODO move into static function in TaskIF

/**
 * The init task is created automatically by RTEMS.
 * As one may need to control it (e.g. suspending it for a while),
 * this dummy class provides an implementation of TaskIF to do so.
 * Warning: The init task is deleted with this stub, i.e. the destructor
 * calls rtems_task_delete(RTEMS_SELF)
 */
class InitTask: public PeriodicTaskIF {
public:
	InitTask();
	virtual ~InitTask();
	ReturnValue_t startTask();

	ReturnValue_t sleepFor(uint32_t ms);

	uint32_t getPeriodMs() const;

	void deleteTask();
};

#endif /* OS_RTEMS_INITTASK_H_ */

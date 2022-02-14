#ifndef OS_RTEMS_INITTASK_H_
#define OS_RTEMS_INITTASK_H_

// TODO move into static function in TaskIF

/**
 * The init task is created automatically by RTEMS.
 * As one may need to control it (e.g. suspending it for a while),
 * this dummy class provides an implementation of TaskIF to do so.
 * Warning: The init task is deleted with this stub, i.e. the destructor
 * calls rtems_task_delete(RTEMS_SELF)
 */
class InitTask {
 public:
  InitTask();
  virtual ~InitTask();
};

#endif /* OS_RTEMS_INITTASK_H_ */

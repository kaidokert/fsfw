#ifndef FRAMEWORK_TASKS_TYPEDEF_H_
#define FRAMEWORK_TASKS_TYPEDEF_H_

//TODO more generic?
typedef const char* TaskName;
typedef uint8_t TaskPriority;
typedef uint16_t TaskStackSize;
typedef double TaskPeriod;
typedef void (*TaskDeadlineMissedFunction)();

#endif /* FRAMEWORK_TASKS_TYPEDEF_H_ */

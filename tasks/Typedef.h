#ifndef FRAMEWORK_TASKS_TYPEDEF_H_
#define FRAMEWORK_TASKS_TYPEDEF_H_

typedef const char* TaskName;
typedef uint8_t TaskPriority;
typedef size_t TaskStackSize;
typedef double TaskPeriod;
typedef void (*TaskDeadlineMissedFunction)();

#endif /* FRAMEWORK_TASKS_TYPEDEF_H_ */

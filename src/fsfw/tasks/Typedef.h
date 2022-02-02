#ifndef FSFW_TASKS_TYPEDEF_H_
#define FSFW_TASKS_TYPEDEF_H_

#include <cstddef>
#include <cstdint>

typedef const char* TaskName;
typedef uint32_t TaskPriority;
typedef size_t TaskStackSize;
typedef double TaskPeriod;
typedef void (*TaskDeadlineMissedFunction)();

#endif /* FSFW_TASKS_TYPEDEF_H_ */

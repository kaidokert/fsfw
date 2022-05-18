#ifndef FSFW_TASKS_TYPEDEF_H_
#define FSFW_TASKS_TYPEDEF_H_

#include <cstddef>
#include <cstdint>

using TaskName = const char*;
using TaskPriority = uint32_t;
using TaskStackSize = size_t;
using TaskPeriod = double;
using TaskDeadlineMissedFunction = void (*)();

#endif /* FSFW_TASKS_TYPEDEF_H_ */

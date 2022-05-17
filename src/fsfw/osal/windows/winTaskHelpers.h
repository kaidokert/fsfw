#include <map>
#include <thread>

#include "../../tasks/TaskFactory.h"

#ifdef _WIN32

namespace tasks {

enum PriorityClass : uint16_t {
  CLASS_IDLE,
  CLASS_BELOW_NORMAL,
  CLASS_NORMAL,
  CLASS_ABOVE_NORMAL,
  CLASS_HIGH,
  CLASS_REALTIME
};
enum PriorityNumber : uint16_t {
  IDLE,
  LOWEST,
  BELOW_NORMAL,
  NORMAL,
  ABOVE_NORMAL,
  HIGHEST,
  CRITICAL
};
TaskPriority makeWinPriority(PriorityClass prioClass = PriorityClass::CLASS_NORMAL,
                             PriorityNumber prioNumber = PriorityNumber::NORMAL);
void getWinPriorityParameters(TaskPriority priority, DWORD& priorityClass, int& priorityNumber);

ReturnValue_t setTaskPriority(HANDLE nativeHandle, TaskPriority priority);

}  // namespace tasks

#endif

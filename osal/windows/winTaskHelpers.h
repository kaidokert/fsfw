#include "../../tasks/TaskFactory.h"

#include <thread>
#include <map>

#ifdef _WIN32

namespace tasks {

enum PriorityClass: uint16_t {
    CLASS_IDLE,
    CLASS_BELOW_NORMAL,
    CLASS_NORMAL,
    CLASS_ABOVE_NORMAL,
    CLASS_HIGH,
    CLASS_REALTIME
};
enum PriorityNumber: uint16_t {
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
void getWinPriorityParameters(TaskPriority priority, DWORD& priorityClass,
        int& priorityNumber);

ReturnValue_t setTaskPriority(HANDLE nativeHandle, TaskPriority priority);

ReturnValue_t insertTaskName(std::thread::id threadId, std::string taskName);
std::string getTaskName(std::thread::id threadId);

}

#endif


#include <fsfw/osal/windows/winTaskHelpers.h>
#include <mutex>

std::mutex nameMapLock;
std::map<std::thread::id, std::string> taskNameMap;

TaskPriority tasks::makeWinPriority(PriorityClass prioClass, PriorityNumber prioNumber) {
    return (static_cast<uint16_t>(prioClass) << 16) | static_cast<uint16_t> (prioNumber);
}

void tasks::getWinPriorityParameters(TaskPriority priority,
        DWORD& priorityClass, int& priorityNumber) {
    PriorityClass classInternal = static_cast<PriorityClass>(priority >> 16 & 0xff);
    PriorityNumber numberInternal = static_cast<PriorityNumber>(priority & 0xff);
    switch(classInternal) {
    case(CLASS_IDLE): {
        priorityClass = IDLE_PRIORITY_CLASS;
        break;
    }
    case(CLASS_BELOW_NORMAL): {
        priorityClass = BELOW_NORMAL_PRIORITY_CLASS;
        break;
    }
    case(CLASS_NORMAL): {
        priorityClass = NORMAL_PRIORITY_CLASS;
        break;
    }
    case(CLASS_ABOVE_NORMAL): {
        priorityClass = ABOVE_NORMAL_PRIORITY_CLASS;
        break;
    }
    case(CLASS_HIGH): {
        priorityClass = HIGH_PRIORITY_CLASS;
        break;
    }
    case(CLASS_REALTIME): {
        priorityClass = REALTIME_PRIORITY_CLASS;
        break;
    }
    default: {
        priorityClass = NORMAL_PRIORITY_CLASS;
    }
    }

    switch(numberInternal) {
    case(IDLE): {
        priorityNumber = THREAD_PRIORITY_IDLE;
        break;
    }
    case(LOWEST): {
        priorityNumber = THREAD_PRIORITY_LOWEST;
        break;
    }
    case(BELOW_NORMAL): {
        priorityNumber = THREAD_PRIORITY_BELOW_NORMAL;
        break;
    }
    case(NORMAL): {
        priorityNumber = THREAD_PRIORITY_NORMAL;
        break;
    }
    case(ABOVE_NORMAL): {
        priorityNumber = THREAD_PRIORITY_ABOVE_NORMAL;
        break;
    }
    case(HIGHEST): {
        priorityNumber = THREAD_PRIORITY_HIGHEST;
        break;
    }
    case(CRITICAL): {
        priorityNumber = THREAD_PRIORITY_TIME_CRITICAL;
        break;
    }
    default: {
        priorityNumber = THREAD_PRIORITY_NORMAL;
    }
    }
}

ReturnValue_t tasks::setTaskPriority(HANDLE nativeHandle, TaskPriority priority) {
    /* List of possible priority classes:
    https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-setpriorityclass
    And respective thread priority numbers:
    https://docs.microsoft.com/en-us/windows/win32/procthread/scheduling-priorities
    */
    DWORD dwPriorityClass = 0;
    int nPriorityNumber = 0;
    tasks::getWinPriorityParameters(priority, dwPriorityClass, nPriorityNumber);
    int result = SetPriorityClass(
            reinterpret_cast<HANDLE>(nativeHandle),
            dwPriorityClass);
    if(result != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "PeriodicTask: Windows SetPriorityClass failed with code "
                << GetLastError() << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
#endif
    }
    result = SetThreadPriority(
            reinterpret_cast<HANDLE>(nativeHandle),
            nPriorityNumber);
    if(result != 0) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
        sif::error << "PeriodicTask: Windows SetPriorityClass failed with code "
                << GetLastError() << std::endl;
        return HasReturnvaluesIF::RETURN_FAILED;
#endif
    }
    return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t tasks::insertTaskName(std::thread::id threadId, std::string taskName) {
    std::lock_guard<std::mutex> lg(nameMapLock);
    auto returnPair = taskNameMap.emplace(threadId, taskName);
    if(not returnPair.second) {
        return HasReturnvaluesIF::RETURN_FAILED;
    }
    return HasReturnvaluesIF::RETURN_OK;
}

std::string tasks::getTaskName(std::thread::id threadId) {
    std::lock_guard<std::mutex> lg(nameMapLock);
    auto resultIter = taskNameMap.find(threadId);
    if(resultIter != taskNameMap.end()) {
        return resultIter->second;
    }
    else {
        return "Unknown task";
    }
}

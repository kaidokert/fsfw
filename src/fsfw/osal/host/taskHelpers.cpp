#include "fsfw/osal/host/taskHelpers.h"

#include <map>
#include <mutex>

std::mutex nameMapLock;
std::map<std::thread::id, std::string> taskNameMap;

ReturnValue_t tasks::insertTaskName(std::thread::id threadId, std::string taskName) {
  std::lock_guard<std::mutex> lg(nameMapLock);
  auto returnPair = taskNameMap.emplace(threadId, taskName);
  if (not returnPair.second) {
    return HasReturnvaluesIF::RETURN_FAILED;
  }
  return HasReturnvaluesIF::RETURN_OK;
}

std::string tasks::getTaskName(std::thread::id threadId) {
  std::lock_guard<std::mutex> lg(nameMapLock);
  auto resultIter = taskNameMap.find(threadId);
  if (resultIter != taskNameMap.end()) {
    return resultIter->second;
  } else {
    return "Unknown task";
  }
}

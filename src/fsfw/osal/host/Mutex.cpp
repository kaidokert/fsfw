#include "fsfw/osal/host/Mutex.h"

#include "fsfw/serviceinterface/ServiceInterface.h"

Mutex::Mutex() {}

ReturnValue_t Mutex::lockMutex(TimeoutType timeoutType, uint32_t timeoutMs) {
  if (timeoutType == TimeoutType::BLOCKING) {
    mutex.lock();
    return HasReturnvaluesIF::RETURN_OK;
  } else if (timeoutType == TimeoutType::POLLING) {
    if (mutex.try_lock()) {
      return HasReturnvaluesIF::RETURN_OK;
    }
  } else if (timeoutType == TimeoutType::WAITING) {
    auto chronoMs = std::chrono::milliseconds(timeoutMs);
    if (mutex.try_lock_for(chronoMs)) {
      return HasReturnvaluesIF::RETURN_OK;
    }
  }
  return MutexIF::MUTEX_TIMEOUT;
}

ReturnValue_t Mutex::unlockMutex() {
  mutex.unlock();
  return HasReturnvaluesIF::RETURN_OK;
}

std::timed_mutex* Mutex::getMutexHandle() { return &mutex; }

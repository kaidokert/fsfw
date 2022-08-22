#include "fsfw/osal/linux/BinarySemaphore.h"

#include <errno.h>
#include <stdio.h>
#include <time.h>

#include <cstring>

#include "fsfw/osal/linux/unixUtility.h"
#include "fsfw/serviceinterface/ServiceInterfacePrinter.h"
#include "fsfw/serviceinterface/ServiceInterfaceStream.h"

BinarySemaphore::BinarySemaphore() {
  // Using unnamed semaphores for now
  initSemaphore();
}

BinarySemaphore::~BinarySemaphore() { sem_destroy(&handle); }

BinarySemaphore::BinarySemaphore(BinarySemaphore&& s) { initSemaphore(); }

BinarySemaphore& BinarySemaphore::operator=(BinarySemaphore&& s) {
  initSemaphore();
  return *this;
}

ReturnValue_t BinarySemaphore::acquire(TimeoutType timeoutType, uint32_t timeoutMs) {
  int result = 0;
  if (timeoutType == TimeoutType::POLLING) {
    result = sem_trywait(&handle);
  } else if (timeoutType == TimeoutType::BLOCKING) {
    result = sem_wait(&handle);
  } else if (timeoutType == TimeoutType::WAITING) {
    timespec timeOut;
    clock_gettime(CLOCK_REALTIME, &timeOut);
    uint64_t nseconds = timeOut.tv_sec * 1000000000 + timeOut.tv_nsec;
    nseconds += timeoutMs * 1000000;
    timeOut.tv_sec = nseconds / 1000000000;
    timeOut.tv_nsec = nseconds - timeOut.tv_sec * 1000000000;
    result = sem_timedwait(&handle, &timeOut);
    if (result != 0 and errno == EINVAL) {
      utility::printUnixErrorGeneric(CLASS_NAME, "acquire", "sem_timedwait");
    }
  }
  if (result == 0) {
    return returnvalue::OK;
  }

  switch (errno) {
    case (EAGAIN):
      // Operation could not be performed without blocking (for sem_trywait)
    case (ETIMEDOUT): {
      // Semaphore is 0
      utility::printUnixErrorGeneric(CLASS_NAME, "acquire", "ETIMEDOUT");
      return SemaphoreIF::SEMAPHORE_TIMEOUT;
    }
    case (EINVAL): {
      // Semaphore invalid
      utility::printUnixErrorGeneric(CLASS_NAME, "acquire", "EINVAL");
      return SemaphoreIF::SEMAPHORE_INVALID;
    }
    case (EINTR): {
      // Call was interrupted by signal handler
      utility::printUnixErrorGeneric(CLASS_NAME, "acquire", "EINTR");
      return returnvalue::FAILED;
    }
    default:
      return returnvalue::FAILED;
  }
}

ReturnValue_t BinarySemaphore::release() { return BinarySemaphore::release(&this->handle); }

ReturnValue_t BinarySemaphore::release(sem_t* handle) {
  ReturnValue_t countResult = checkCount(handle, 1);
  if (countResult != returnvalue::OK) {
    return countResult;
  }

  int result = sem_post(handle);
  if (result == 0) {
    return returnvalue::OK;
  }

  switch (errno) {
    case (EINVAL): {
      // Semaphore invalid
      utility::printUnixErrorGeneric(CLASS_NAME, "release", "EINVAL");
      return SemaphoreIF::SEMAPHORE_INVALID;
    }
    case (EOVERFLOW): {
      // SEM_MAX_VALUE overflow. This should never happen
      utility::printUnixErrorGeneric(CLASS_NAME, "release", "EOVERFLOW");
      return returnvalue::FAILED;
    }
    default:
      return returnvalue::FAILED;
  }
}

uint8_t BinarySemaphore::getSemaphoreCounter() const {
  // And another ugly cast :-D
  return getSemaphoreCounter(const_cast<sem_t*>(&this->handle));
}

uint8_t BinarySemaphore::getSemaphoreCounter(sem_t* handle) {
  int value = 0;
  int result = sem_getvalue(handle, &value);
  if (result == 0) {
    return value;
  } else if (result != 0 and errno == EINVAL) {
    // Could be called from interrupt, use lightweight printf
    sif::printError(
        "BinarySemaphore::getSemaphoreCounter: "
        "Invalid semaphore\n");
    return 0;
  } else {
    // This should never happen.
    return 0;
  }
}

void BinarySemaphore::initSemaphore(uint8_t initCount) {
  auto result = sem_init(&handle, true, initCount);
  if (result == -1) {
    switch (errno) {
      case (EINVAL): {
        utility::printUnixErrorGeneric(CLASS_NAME, "initSemaphore", "EINVAL");
        break;
      }
      case (ENOSYS): {
        // System does not support process-shared semaphores
        utility::printUnixErrorGeneric(CLASS_NAME, "initSemaphore", "ENOSYS");
        break;
      }
    }
  }
}

ReturnValue_t BinarySemaphore::checkCount(sem_t* handle, uint8_t maxCount) {
  int value = getSemaphoreCounter(handle);
  if (value >= maxCount) {
    if (maxCount == 1 and value > 1) {
      // Binary Semaphore special case.
      // This is a config error use lightweight printf is this is called
      // from an interrupt
      printf("BinarySemaphore::release: Value of binary semaphore greater than 1!\n");
      return returnvalue::FAILED;
    }
    return SemaphoreIF::SEMAPHORE_NOT_OWNED;
  }
  return returnvalue::OK;
}

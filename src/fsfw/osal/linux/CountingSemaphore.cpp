#include "fsfw/osal/linux/CountingSemaphore.h"

#include <errno.h>

#include "fsfw/osal/linux/unixUtility.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

CountingSemaphore::CountingSemaphore(const uint8_t maxCount, uint8_t initCount)
    : maxCount(maxCount), initCount(initCount) {
  if (initCount > maxCount) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::warning << "CountingSemaphoreUsingTask: Max count bigger than "
                    "intial cout. Setting initial count to max count"
                 << std::endl;
#else
    sif::printWarning(
        "CountingSemaphoreUsingTask: Max count bigger than "
        "intial cout. Setting initial count to max count\n");
#endif
    initCount = maxCount;
  }

  initSemaphore(initCount);
}

CountingSemaphore::CountingSemaphore(CountingSemaphore&& other)
    : maxCount(other.maxCount), initCount(other.initCount) {
  initSemaphore(initCount);
}

CountingSemaphore& CountingSemaphore::operator=(CountingSemaphore&& other) {
  initSemaphore(other.initCount);
  return *this;
}

ReturnValue_t CountingSemaphore::release() {
  ReturnValue_t result = checkCount(&handle, maxCount);
  if (result != returnvalue::OK) {
    return result;
  }
  return CountingSemaphore::release(&this->handle);
}

ReturnValue_t CountingSemaphore::release(sem_t* handle) {
  int result = sem_post(handle);
  if (result == 0) {
    return returnvalue::OK;
  }

  switch (errno) {
    case (EINVAL): {
      // Semaphore invalid
      utility::printUnixErrorGeneric("CountingSemaphore", "release", "EINVAL");
      return SemaphoreIF::SEMAPHORE_INVALID;
    }

    case (EOVERFLOW): {
      // SEM_MAX_VALUE overflow. This should never happen
      utility::printUnixErrorGeneric("CountingSemaphore", "release", "EOVERFLOW");
      return SemaphoreIF::SEMAPHORE_INVALID;
    }

    default:
      return returnvalue::FAILED;
  }
}

uint8_t CountingSemaphore::getMaxCount() const { return maxCount; }

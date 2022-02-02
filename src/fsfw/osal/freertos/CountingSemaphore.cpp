#include "fsfw/osal/freertos/CountingSemaphore.h"

#include "FreeRTOS.h"
#include "fsfw/osal/freertos/TaskManagement.h"
#include "fsfw/serviceinterface/ServiceInterface.h"
#include "semphr.h"

// Make sure #define configUSE_COUNTING_SEMAPHORES 1 is set in
// free FreeRTOSConfig.h file.
CountingSemaphore::CountingSemaphore(const uint8_t maxCount, uint8_t initCount)
    : maxCount(maxCount), initCount(initCount) {
  if (initCount > maxCount) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "CountingSemaphoreUsingTask: Max count bigger than "
                  "intial cout. Setting initial count to max count."
               << std::endl;
#endif
    initCount = maxCount;
  }

  handle = xSemaphoreCreateCounting(maxCount, initCount);
  if (handle == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "CountingSemaphore: Creation failure" << std::endl;
#endif
  }
}

CountingSemaphore::CountingSemaphore(CountingSemaphore&& other)
    : maxCount(other.maxCount), initCount(other.initCount) {
  handle = xSemaphoreCreateCounting(other.maxCount, other.initCount);
  if (handle == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "CountingSemaphore: Creation failure" << std::endl;
#endif
  }
}

CountingSemaphore& CountingSemaphore::operator=(CountingSemaphore&& other) {
  handle = xSemaphoreCreateCounting(other.maxCount, other.initCount);
  if (handle == nullptr) {
#if FSFW_CPP_OSTREAM_ENABLED == 1
    sif::error << "CountingSemaphore: Creation failure" << std::endl;
#endif
  }
  return *this;
}

uint8_t CountingSemaphore::getMaxCount() const { return maxCount; }

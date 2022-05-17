#include "BinarySemaphore.h"

#include <rtems/rtems/sem.h>

BinarySemaphore::BinarySemaphore() {}

BinarySemaphore::~BinarySemaphore() {}

ReturnValue_t BinarySemaphore::acquire(TimeoutType timeoutType, uint32_t timeoutMs) {
  return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t BinarySemaphore::release() { return HasReturnvaluesIF::RETURN_OK; }

uint8_t BinarySemaphore::getSemaphoreCounter() const { return 0; }

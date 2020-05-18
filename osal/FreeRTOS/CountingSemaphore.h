#ifndef FRAMEWORK_OSAL_FREERTOS_COUNTINGSEMAPHORE_H_
#define FRAMEWORK_OSAL_FREERTOS_COUNTINGSEMAPHORE_H_
#include <framework/osal/FreeRTOS/BinarySemaphore.h>

class CountingSemaphore: public Semaphore {
public:
	CountingSemaphore(uint8_t count, uint8_t initCount);
	//! @brief Copy ctor, disabled
	CountingSemaphore(const CountingSemaphore&) = delete;
	//! @brief Copy assignment, disabled
	CountingSemaphore& operator=(const CountingSemaphore&) = delete;
	//! @brief Move ctor
	CountingSemaphore (CountingSemaphore &&);
	//! @brief Move assignment
	CountingSemaphore & operator=(CountingSemaphore &&);
private:
	uint8_t count = 0;
	uint8_t initCount = 0;
};

#endif /* FRAMEWORK_OSAL_FREERTOS_COUNTINGSEMAPHORE_H_ */

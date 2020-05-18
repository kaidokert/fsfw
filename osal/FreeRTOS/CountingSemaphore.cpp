#include <framework/osal/FreeRTOS/CountingSemaphore.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

CountingSemaphore::CountingSemaphore(uint8_t count, uint8_t initCount):
		count(count), initCount(initCount) {
	handle = xSemaphoreCreateCounting(count, initCount);
	if(handle == nullptr) {
		sif::error << "CountingSemaphore: Creation failure" << std::endl;
	}
}

CountingSemaphore::CountingSemaphore(CountingSemaphore&& other) {
	handle = xSemaphoreCreateCounting(other.count, other.initCount);
	if(handle == nullptr) {
		sif::error << "CountingSemaphore: Creation failure" << std::endl;
	}
}

CountingSemaphore& CountingSemaphore::operator =(
		CountingSemaphore&& other) {
	handle = xSemaphoreCreateCounting(other.count, other.initCount);
	if(handle == nullptr) {
		sif::error << "CountingSemaphore: Creation failure" << std::endl;
	}
	return * this;
}

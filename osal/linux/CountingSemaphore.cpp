#include "../../osal/linux/CountingSemaphore.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"

CountingSemaphore::CountingSemaphore(const uint8_t maxCount, uint8_t initCount):
		maxCount(maxCount), initCount(initCount) {
	if(initCount > maxCount) {
		sif::error << "CountingSemaphoreUsingTask: Max count bigger than "
				"intial cout. Setting initial count to max count." << std::endl;
		initCount = maxCount;
	}

	initSemaphore(initCount);
}

CountingSemaphore::CountingSemaphore(CountingSemaphore&& other):
		maxCount(other.maxCount), initCount(other.initCount) {
	initSemaphore(initCount);
}

CountingSemaphore& CountingSemaphore::operator =(
		CountingSemaphore&& other) {
	initSemaphore(other.initCount);
	return * this;
}

ReturnValue_t CountingSemaphore::release() {
	ReturnValue_t result = checkCount(&handle, maxCount);
	if(result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	return CountingSemaphore::release(&this->handle);
}

ReturnValue_t CountingSemaphore::release(sem_t* handle) {
	int result = sem_post(handle);
	if(result == 0) {
		return HasReturnvaluesIF::RETURN_OK;
	}

	switch(errno) {
	case(EINVAL):
		// Semaphore invalid
		return SemaphoreIF::SEMAPHORE_INVALID;
	case(EOVERFLOW):
		// SEM_MAX_VALUE overflow. This should never happen
	default:
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

uint8_t CountingSemaphore::getMaxCount() const {
	return maxCount;
}


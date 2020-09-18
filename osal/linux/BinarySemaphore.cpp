#include "../../osal/linux/BinarySemaphore.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"

extern "C" {
#include <errno.h>
#include <string.h>
}

BinarySemaphore::BinarySemaphore() {
	// Using unnamed semaphores for now
	initSemaphore();
}

BinarySemaphore::~BinarySemaphore() {
	sem_destroy(&handle);
}

BinarySemaphore::BinarySemaphore(BinarySemaphore&& s) {
	initSemaphore();
}

BinarySemaphore& BinarySemaphore::operator =(
        BinarySemaphore&& s) {
	initSemaphore();
	return * this;
}

ReturnValue_t BinarySemaphore::acquire(TimeoutType timeoutType,
		 uint32_t timeoutMs) {
	int result = 0;
	if(timeoutType == TimeoutType::POLLING) {
		result = sem_trywait(&handle);
	}
	else if(timeoutType == TimeoutType::BLOCKING) {
	    result = sem_wait(&handle);
	}
	else if(timeoutType == TimeoutType::WAITING){
		timespec timeOut;
		clock_gettime(CLOCK_REALTIME, &timeOut);
		uint64_t nseconds = timeOut.tv_sec * 1000000000 + timeOut.tv_nsec;
		nseconds += timeoutMs * 1000000;
		timeOut.tv_sec = nseconds / 1000000000;
		timeOut.tv_nsec = nseconds - timeOut.tv_sec * 1000000000;
	    result = sem_timedwait(&handle, &timeOut);
	    if(result != 0 and errno == EINVAL) {
	    	sif::debug << "BinarySemaphore::acquire: Invalid time value possible"
	    			<< std::endl;
	    }
	}
	if(result == 0) {
		return HasReturnvaluesIF::RETURN_OK;
	}

	switch(errno) {
	case(EAGAIN):
		// Operation could not be performed without blocking (for sem_trywait)
	case(ETIMEDOUT):
		// Semaphore is 0
		return SemaphoreIF::SEMAPHORE_TIMEOUT;
	case(EINVAL):
		// Semaphore invalid
		return SemaphoreIF::SEMAPHORE_INVALID;
	case(EINTR):
		// Call was interrupted by signal handler
		sif::debug << "BinarySemaphore::acquire: Signal handler interrupted."
				"Code " << strerror(errno) << std::endl;
		/* No break */
	default:
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

ReturnValue_t BinarySemaphore::release() {
	return BinarySemaphore::release(&this->handle);
}

ReturnValue_t BinarySemaphore::release(sem_t *handle) {
	ReturnValue_t countResult = checkCount(handle, 1);
	if(countResult != HasReturnvaluesIF::RETURN_OK) {
		return countResult;
	}

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

uint8_t BinarySemaphore::getSemaphoreCounter() const {
	// And another ugly cast :-D
	return getSemaphoreCounter(const_cast<sem_t*>(&this->handle));
}

uint8_t BinarySemaphore::getSemaphoreCounter(sem_t *handle) {
	int value = 0;
	int result = sem_getvalue(handle, &value);
	if (result == 0) {
		return value;
	}
	else if(result != 0 and errno == EINVAL) {
		// Could be called from interrupt, use lightweight printf
		printf("BinarySemaphore::getSemaphoreCounter: Invalid semaphore\n");
		return 0;
	}
	else {
		// This should never happen.
		return 0;
	}
}

void BinarySemaphore::initSemaphore(uint8_t initCount) {
	auto result = sem_init(&handle, true, initCount);
	if(result == -1) {
		switch(errno) {
		case(EINVAL):
			// Value exceeds SEM_VALUE_MAX
		case(ENOSYS):
		// System does not support process-shared semaphores
		sif::error << "BinarySemaphore: Init failed with" << strerror(errno)
				<< std::endl;
		}
	}
}

ReturnValue_t BinarySemaphore::checkCount(sem_t* handle, uint8_t maxCount) {
	int value = getSemaphoreCounter(handle);
	if(value >= maxCount) {
		if(maxCount == 1 and value > 1) {
			// Binary Semaphore special case.
			// This is a config error use lightweight printf is this is called
			// from an interrupt
			printf("BinarySemaphore::release: Value of binary semaphore greater"
					" than 1!\n");
			return HasReturnvaluesIF::RETURN_FAILED;
		}
		return SemaphoreIF::SEMAPHORE_NOT_OWNED;
	}
	return HasReturnvaluesIF::RETURN_OK;
}

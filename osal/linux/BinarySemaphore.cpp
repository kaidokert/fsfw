#include <framework/osal/linux/BinarySemaphore.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

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

ReturnValue_t BinarySemaphore::acquire(uint32_t timeoutMs) {
	int result = 0;
	if(timeoutMs == SemaphoreIF::NO_TIMEOUT) {
		result = sem_trywait(&handle);
	}
	if(timeoutMs == SemaphoreIF::MAX_TIMEOUT) {
	    result = sem_wait(&handle);
	}
	else if(timeoutMs > SemaphoreIF::NO_TIMEOUT){
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

	switch(result) {
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
		sif::debug << "BinarySemaphore::acquire: Signal handler interrupted"
				<< std::endl;
		/* No break */
	default:
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

ReturnValue_t BinarySemaphore::release() {
	return release(&this->handle);
}

ReturnValue_t BinarySemaphore::release(sem_t *handle) {
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
		sif::debug << "BInarySemaphore::getSemaphoreCounter: Invalid"
				" Semaphore." << std::endl;
		return 0;
	}
	else {
		// This should never happen.
		return 0;
	}
}

void BinarySemaphore::initSemaphore() {
	auto result = sem_init(&handle, true, 1);
	if(result == -1) {
		switch(errno) {
		case(EINVAL):
						// Value excees SEM_VALUE_MAX
		case(ENOSYS):
		// System does not support process-shared semaphores
		sif::error << "BinarySemaphore: Init failed with" << strerror(errno)
		<< std::endl;
		}
	}
}

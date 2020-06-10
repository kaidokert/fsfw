#include <framework/osal/linux/Mutex.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <framework/timemanager/Clock.h>

const uint32_t MutexIF::NO_TIMEOUT = 0;
uint8_t Mutex::count = 0;


#include <cstring>
#include <errno.h>

Mutex::Mutex() {
	pthread_mutexattr_t mutexAttr;
	int status = pthread_mutexattr_init(&mutexAttr);
	if (status != 0) {
		sif::error << "Mutex: Attribute init failed with: " << strerror(status) << std::endl;
	}
	status = pthread_mutexattr_setprotocol(&mutexAttr, PTHREAD_PRIO_INHERIT);
	if (status != 0) {
		sif::error << "Mutex: Attribute set PRIO_INHERIT failed with: " << strerror(status)
				<< std::endl;
	}
	status = pthread_mutex_init(&mutex, &mutexAttr);
	if (status != 0) {
		sif::error << "Mutex: creation with name, id " << mutex.__data.__count
				<< ", " << " failed with " << strerror(status) << std::endl;
	}
	//After a mutex attributes object has been used to initialize one or more mutexes, any function affecting the attributes object (including destruction) shall not affect any previously initialized mutexes.
	status = pthread_mutexattr_destroy(&mutexAttr);
	if (status != 0) {
		sif::error << "Mutex: Attribute destroy failed with " << strerror(status) << std::endl;
	}
}

Mutex::~Mutex() {
	//No Status check yet
	pthread_mutex_destroy(&mutex);
}

ReturnValue_t Mutex::lockMutex(uint32_t timeoutMs) {
	int status = 0;
	if (timeoutMs != MutexIF::NO_TIMEOUT) {
		timespec timeOut;
		clock_gettime(CLOCK_REALTIME, &timeOut);
		uint64_t nseconds = timeOut.tv_sec * 1000000000 + timeOut.tv_nsec;
		nseconds += timeoutMs * 1000000;
		timeOut.tv_sec = nseconds / 1000000000;
		timeOut.tv_nsec = nseconds - timeOut.tv_sec * 1000000000;
		status = pthread_mutex_timedlock(&mutex, &timeOut);
	} else {
		status = pthread_mutex_lock(&mutex);
	}
	switch (status) {
	case EINVAL:
		//The mutex was created with the protocol attribute having the value PTHREAD_PRIO_PROTECT and the calling thread's priority is higher than the mutex's current priority ceiling.
		return WRONG_ATTRIBUTE_SETTING;
		//The process or thread would have blocked, and the abs_timeout parameter specified a nanoseconds field value less than zero or greater than or equal to 1000 million.
		//The value specified by mutex does not refer to an initialized mutex object.
		//return MUTEX_NOT_FOUND;
	case EBUSY:
		//The mutex could not be acquired because it was already locked.
		return MUTEX_ALREADY_LOCKED;
	case ETIMEDOUT:
		//The mutex could not be locked before the specified timeout expired.
		return MUTEX_TIMEOUT;
	case EAGAIN:
		//The mutex could not be acquired because the maximum number of recursive locks for mutex has been exceeded.
		return MUTEX_MAX_LOCKS;
	case EDEADLK:
		//A deadlock condition was detected or the current thread already owns the mutex.
		return CURR_THREAD_ALREADY_OWNS_MUTEX;
	case 0:
		//Success
		return HasReturnvaluesIF::RETURN_OK;
	default:
		return HasReturnvaluesIF::RETURN_FAILED;
	};
}

ReturnValue_t Mutex::unlockMutex() {
	int status = pthread_mutex_unlock(&mutex);
	switch (status) {
	case EINVAL:
		//The value specified by mutex does not refer to an initialized mutex object.
		return MUTEX_NOT_FOUND;
	case EAGAIN:
		//The mutex could not be acquired because the maximum number of recursive locks for mutex has been exceeded.
		return MUTEX_MAX_LOCKS;
	case EPERM:
		//The current thread does not own the mutex.
		return CURR_THREAD_DOES_NOT_OWN_MUTEX;
	case 0:
		//Success
		return HasReturnvaluesIF::RETURN_OK;
	default:
		return HasReturnvaluesIF::RETURN_FAILED;
	};
}

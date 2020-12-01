#include "Mutex.h"
#include "../../serviceinterface/ServiceInterfaceStream.h"

uint8_t Mutex::count = 0;

Mutex::Mutex() :
		mutexId(0) {
	rtems_name mutexName = ('M' << 24) + ('T' << 16) + ('X' << 8) + count++;
	rtems_status_code status = rtems_semaphore_create(mutexName, 1,
	RTEMS_BINARY_SEMAPHORE | RTEMS_PRIORITY | RTEMS_INHERIT_PRIORITY, 0,
			&mutexId);
	if (status != RTEMS_SUCCESSFUL) {
		sif::error << "Mutex: creation with name, id " << mutexName << ", " << mutexId
				<< " failed with " << status << std::endl;
	}
}

Mutex::~Mutex() {
	rtems_status_code status = rtems_semaphore_delete(mutexId);
	if (status != RTEMS_SUCCESSFUL) {
		sif::error << "Mutex: deletion for id " << mutexId
				<< " failed with " << status << std::endl;
	}
}

ReturnValue_t Mutex::lockMutex(TimeoutType timeoutType =
        TimeoutType::BLOCKING, uint32_t timeoutMs) {
	rtems_status_code status = RTEMS_INVALID_ID;
	if(timeoutMs == MutexIF::TimeoutType::BLOCKING) {
		status = rtems_semaphore_obtain(mutexId,
				RTEMS_WAIT, RTEMS_NO_TIMEOUT);
	}
	else if(timeoutMs == MutexIF::TimeoutType::POLLING) {
		timeoutMs = RTEMS_NO_TIMEOUT;
		status = rtems_semaphore_obtain(mutexId,
				RTEMS_NO_WAIT, 0);
	}
	else {
		status = rtems_semaphore_obtain(mutexId,
				RTEMS_WAIT, timeoutMs);
	}

	switch(status){
	case RTEMS_SUCCESSFUL:
		//semaphore obtained successfully
		return HasReturnvaluesIF::RETURN_OK;
	case RTEMS_UNSATISFIED:
		//semaphore not available
		return MUTEX_NOT_FOUND;
	case RTEMS_TIMEOUT:
		//timed out waiting for semaphore
		return MUTEX_TIMEOUT;
	case RTEMS_OBJECT_WAS_DELETED:
		//semaphore deleted while waiting
		return MUTEX_DESTROYED_WHILE_WAITING;
	case RTEMS_INVALID_ID:
		//invalid semaphore id
		return MUTEX_INVALID_ID;
	default:
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

ReturnValue_t Mutex::unlockMutex() {
	rtems_status_code status = rtems_semaphore_release(mutexId);
	switch(status){
	case RTEMS_SUCCESSFUL:
		//semaphore obtained successfully
		return HasReturnvaluesIF::RETURN_OK;
	case RTEMS_NOT_OWNER_OF_RESOURCE:
		//semaphore not available
		return CURR_THREAD_DOES_NOT_OWN_MUTEX;
	case RTEMS_INVALID_ID:
		//invalid semaphore id
		return MUTEX_INVALID_ID;
	default:
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}

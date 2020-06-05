#include <framework/osal/host/Mutex.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

const uint32_t MutexIF::NO_TIMEOUT = 0;
const uint32_t MutexIF::MAX_TIMEOUT = 0xffffffff;

ReturnValue_t Mutex::lockMutex(uint32_t timeoutMs) {
	if(timeoutMs == MutexIF::MAX_TIMEOUT) {
		mutex.lock();
		locked = true;
		return HasReturnvaluesIF::RETURN_OK;
	}
	else if(timeoutMs == MutexIF::NO_TIMEOUT) {
		if(mutex.try_lock()) {
			locked = true;
			return HasReturnvaluesIF::RETURN_OK;
		}
	}
	else if(timeoutMs > MutexIF::NO_TIMEOUT){
		auto chronoMs = std::chrono::milliseconds(timeoutMs);
		if(mutex.try_lock_for(chronoMs)) {
			locked = true;
			return HasReturnvaluesIF::RETURN_OK;
		}
	}
	return MutexIF::MUTEX_TIMEOUT;
}

ReturnValue_t Mutex::unlockMutex() {
	if(not locked) {
		return MutexIF::CURR_THREAD_DOES_NOT_OWN_MUTEX;
	}
	mutex.unlock();
	locked = false;
	return HasReturnvaluesIF::RETURN_OK;
}

std::timed_mutex* Mutex::getMutexHandle() {
	return &mutex;
}

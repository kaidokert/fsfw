#include <framework/container/SharedRingBuffer.h>
#include <framework/ipc/MutexFactory.h>
#include <framework/ipc/MutexHelper.h>

SharedRingBuffer::SharedRingBuffer(object_id_t objectId, const size_t size,
		bool overwriteOld, size_t maxExcessBytes):
		SystemObject(objectId), SimpleRingBuffer(size, overwriteOld,
		maxExcessBytes) {
	mutex = MutexFactory::instance()->createMutex();
}

SharedRingBuffer::SharedRingBuffer(object_id_t objectId, uint8_t *buffer,
		const size_t size, bool overwriteOld, size_t maxExcessBytes):
		SystemObject(objectId), SimpleRingBuffer(buffer, size, overwriteOld,
		maxExcessBytes) {
	mutex = MutexFactory::instance()->createMutex();
}

ReturnValue_t SharedRingBuffer::lockRingBufferMutex(
        MutexIF::TimeoutType timeoutType, dur_millis_t timeout) {
    return mutex->lockMutex(timeoutType, timeout);
}

ReturnValue_t SharedRingBuffer::unlockRingBufferMutex() {
    return mutex->unlockMutex();
}

MutexIF* SharedRingBuffer::getMutexHandle() const {
    return mutex;
}

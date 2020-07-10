#include <framework/container/SharedRingBuffer.h>
#include <framework/ipc/MutexFactory.h>
#include <framework/ipc/MutexHelper.h>

SharedRingBuffer::SharedRingBuffer(object_id_t objectId, const size_t size,
		bool overwriteOld, dur_millis_t mutexTimeout):
		SystemObject(objectId), SimpleRingBuffer(size, overwriteOld),
		mutexTimeout(mutexTimeout) {
	mutex = MutexFactory::instance()->createMutex();
}

SharedRingBuffer::SharedRingBuffer(object_id_t objectId, uint8_t *buffer,
		const size_t size, bool overwriteOld, dur_millis_t mutexTimeout):
		SystemObject(objectId), SimpleRingBuffer(buffer, size, overwriteOld),
		mutexTimeout(mutexTimeout) {
	mutex = MutexFactory::instance()->createMutex();
}

ReturnValue_t SharedRingBuffer::writeDataProtected(const uint8_t *data,
		size_t amount) {
	MutexHelper(mutex, mutexTimeout);
	return SimpleRingBuffer::writeData(data,amount);
}

ReturnValue_t SharedRingBuffer::readDataProtected(uint8_t *data, size_t amount,
		bool incrementReadPtr, bool readRemaining,
		size_t *trueAmount) {
	MutexHelper(mutex, mutexTimeout);
	return SimpleRingBuffer::readData(data,amount, incrementReadPtr,
			readRemaining, trueAmount);
}

ReturnValue_t SharedRingBuffer::deleteDataProtected(size_t amount,
		bool deleteRemaining, size_t *trueAmount) {
	MutexHelper(mutex, mutexTimeout);
	return SimpleRingBuffer::deleteData(amount, deleteRemaining, trueAmount);
}

size_t SharedRingBuffer::getAvailableReadDataProtected(uint8_t n) const {
	MutexHelper(mutex, mutexTimeout);
	return ((write + size) - read[n]) % size;
}

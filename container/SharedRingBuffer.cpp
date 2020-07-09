#include <framework/container/SharedRingBuffer.h>
#include <framework/ipc/MutexFactory.h>
#include <framework/ipc/MutexHelper.h>

SharedRingBuffer::SharedRingBuffer(const size_t size, bool overwriteOld):
		SimpleRingBuffer(size, overwriteOld) {
	mutex = MutexFactory::instance()->createMutex();
}

SharedRingBuffer::SharedRingBuffer(uint8_t *buffer, const size_t size,
		bool overwriteOld): SimpleRingBuffer(buffer, size, overwriteOld) {
	mutex = MutexFactory::instance()->createMutex();
}

ReturnValue_t SharedRingBuffer::writeDataProtected(const uint8_t *data,
		size_t amount, dur_millis_t timeout) {
	MutexHelper(mutex, timeout);
	return writeData(data,amount);
}

ReturnValue_t SharedRingBuffer::readDataProtected(uint8_t *data, size_t amount,
		dur_millis_t timeout, bool incrementReadPtr, bool readRemaining,
		size_t *trueAmount) {
	MutexHelper(mutex, timeout);
	return readData(data,amount, incrementReadPtr, readRemaining, trueAmount);
}

ReturnValue_t SharedRingBuffer::deleteDataProtected(size_t amount,
		bool deleteRemaining, size_t *trueAmount, dur_millis_t timeout) {
	MutexHelper(mutex, timeout);
	return deleteData(amount, deleteRemaining, trueAmount);
}

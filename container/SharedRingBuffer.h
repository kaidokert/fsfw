#ifndef FRAMEWORK_CONTAINER_SHAREDRINGBUFFER_H_
#define FRAMEWORK_CONTAINER_SHAREDRINGBUFFER_H_
#include <framework/container/SimpleRingBuffer.h>
#include <framework/ipc/MutexIF.h>
#include <framework/objectmanager/SystemObject.h>
#include <framework/timemanager/Clock.h>

class SharedRingBuffer: public SystemObject,
		public SimpleRingBuffer {
public:
	/**
	 * This constructor allocates a new internal buffer with the supplied size.
	 * @param size
	 * @param overwriteOld
	 * If the ring buffer is overflowing at a write operartion, the oldest data
	 * will be overwritten.
	 */
	SharedRingBuffer(object_id_t objectId, const size_t size,
			bool overwriteOld, dur_millis_t mutexTimeout = 10);

	/**
	 * This constructor takes an external buffer with the specified size.
	 * @param buffer
	 * @param size
	 * @param overwriteOld
	 * If the ring buffer is overflowing at a write operartion, the oldest data
	 * will be overwritten.
	 */
	SharedRingBuffer(object_id_t objectId, uint8_t* buffer, const size_t size,
			bool overwriteOld, dur_millis_t mutexTimeout = 10);

	void setMutexTimeout(dur_millis_t newTimeout);

	/** Performs mutex protected SimpleRingBuffer::writeData call */
	ReturnValue_t writeDataProtected(const uint8_t* data, size_t amount);

	/** Performs mutex protected SimpleRingBuffer::readData call */
	ReturnValue_t readDataProtected(uint8_t *data, size_t amount,
			bool incrementReadPtr = false,
			bool readRemaining = false, size_t *trueAmount = nullptr);

	/** Performs mutex protected SimpleRingBuffer::deleteData call */
	ReturnValue_t deleteDataProtected(size_t amount,
			bool deleteRemaining = false, size_t* trueAmount = nullptr);

	size_t getAvailableReadDataProtected (uint8_t n = 0) const;
private:
	dur_millis_t mutexTimeout;
	MutexIF* mutex = nullptr;
};



#endif /* FRAMEWORK_CONTAINER_SHAREDRINGBUFFER_H_ */

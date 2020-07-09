#ifndef FRAMEWORK_CONTAINER_SHAREDRINGBUFFER_H_
#define FRAMEWORK_CONTAINER_SHAREDRINGBUFFER_H_
#include <framework/container/SimpleRingBuffer.h>
#include <framework/ipc/MutexIF.h>
#include <framework/timemanager/Clock.h>

class SharedRingBuffer: public SimpleRingBuffer {
public:
	/**
	 * This constructor allocates a new internal buffer with the supplied size.
	 * @param size
	 * @param overwriteOld
	 * If the ring buffer is overflowing at a write operartion, the oldest data
	 * will be overwritten.
	 */
	SharedRingBuffer(const size_t size, bool overwriteOld);

	/**
	 * This constructor takes an external buffer with the specified size.
	 * @param buffer
	 * @param size
	 * @param overwriteOld
	 * If the ring buffer is overflowing at a write operartion, the oldest data
	 * will be overwritten.
	 */
	SharedRingBuffer(uint8_t* buffer, const size_t size, bool overwriteOld);

	ReturnValue_t writeDataProtected(const uint8_t* data, size_t amount,
			dur_millis_t timeout = 10);
	ReturnValue_t readDataProtected(uint8_t *data, size_t amount,
			dur_millis_t timeout = 10, bool incrementReadPtr = false,
			bool readRemaining = false, size_t *trueAmount = nullptr);
	ReturnValue_t deleteDataProtected(size_t amount,
			bool deleteRemaining = false, size_t* trueAmount = nullptr,
			dur_millis_t timeout = 10);
private:
	MutexIF* mutex = nullptr;
};



#endif /* FRAMEWORK_CONTAINER_SHAREDRINGBUFFER_H_ */

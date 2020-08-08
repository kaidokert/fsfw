#ifndef FRAMEWORK_CONTAINER_SHAREDRINGBUFFER_H_
#define FRAMEWORK_CONTAINER_SHAREDRINGBUFFER_H_

#include <framework/container/SimpleRingBuffer.h>
#include <framework/ipc/MutexIF.h>
#include <framework/objectmanager/SystemObject.h>
#include <framework/timemanager/Clock.h>

/**
 * @brief   Ring buffer which can be shared among multiple objects
 * @details
 * This class offers a mutex to perform thread-safe operation on the ring
 * buffer. It is still up to the developer to actually perform the lock
 * and unlock operations.
 */
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
			bool overwriteOld, size_t maxExcessBytes,
			dur_millis_t mutexTimeout = 10);

	/**
	 * This constructor takes an external buffer with the specified size.
	 * @param buffer
	 * @param size
	 * @param overwriteOld
	 * If the ring buffer is overflowing at a write operartion, the oldest data
	 * will be overwritten.
	 */
	SharedRingBuffer(object_id_t objectId, uint8_t* buffer, const size_t size,
			bool overwriteOld, size_t maxExcessBytes,
			dur_millis_t mutexTimeout = 10);

	/**
	 * Unless a read-only constant value is read, all operations on the
	 * shared ring buffer should be protected by calling this function.
	 * @param timeoutType
	 * @param timeout
	 * @return
	 */
	virtual ReturnValue_t lockRingBufferMutex(MutexIF::TimeoutType timeoutType,
	        dur_millis_t timeout);
	/**
	 * Any locked mutex also has to be unlocked, otherwise, access to the
	 * shared ring buffer will be blocked.
	 * @return
	 */
	virtual ReturnValue_t unlockRingBufferMutex();

	MutexIF* getMutexHandle() const;
private:
	MutexIF* mutex = nullptr;
};



#endif /* FRAMEWORK_CONTAINER_SHAREDRINGBUFFER_H_ */

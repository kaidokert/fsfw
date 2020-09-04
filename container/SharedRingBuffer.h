#ifndef FRAMEWORK_CONTAINER_SHAREDRINGBUFFER_H_
#define FRAMEWORK_CONTAINER_SHAREDRINGBUFFER_H_

#include "../container/SimpleRingBuffer.h"
#include "../ipc/MutexIF.h"
#include "../objectmanager/SystemObject.h"
#include "../timemanager/Clock.h"

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
			bool overwriteOld, size_t maxExcessBytes);

	/**
	 * This constructor takes an external buffer with the specified size.
	 * @param buffer
	 * @param size
	 * @param overwriteOld
	 * If the ring buffer is overflowing at a write operartion, the oldest data
	 * will be overwritten.
	 */
	SharedRingBuffer(object_id_t objectId, uint8_t* buffer, const size_t size,
			bool overwriteOld, size_t maxExcessBytes);

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

	/**
	 * The mutex handle can be accessed directly, for example to perform
	 * the lock with the #MutexHelper for a RAII compliant lock operation.
	 * @return
	 */
	MutexIF* getMutexHandle() const;
private:
	MutexIF* mutex = nullptr;
};



#endif /* FRAMEWORK_CONTAINER_SHAREDRINGBUFFER_H_ */

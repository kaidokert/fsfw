#ifndef FSFW_CONTAINER_SHAREDRINGBUFFER_H_
#define FSFW_CONTAINER_SHAREDRINGBUFFER_H_

#include "DynamicFIFO.h"
#include "SimpleRingBuffer.h"
#include "fsfw/ipc/MutexIF.h"
#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/timemanager/Clock.h"

/**
 * @brief   Ring buffer which can be shared among multiple objects
 * @details
 * This class offers a mutex to perform thread-safe operation on the ring
 * buffer. It is still up to the developer to actually perform the lock
 * and unlock operations.
 */
class SharedRingBuffer : public SystemObject, public SimpleRingBuffer {
 public:
  /**
   * This constructor allocates a new internal buffer with the supplied size.
   * @param size
   * @param overwriteOld
   * If the ring buffer is overflowing at a write operartion, the oldest data
   * will be overwritten.
   */
  SharedRingBuffer(object_id_t objectId, const size_t size, bool overwriteOld,
                   size_t maxExcessBytes);
  /**
   * This constructor takes an external buffer with the specified size.
   * @param buffer
   * @param size
   * @param overwriteOld
   * If the ring buffer is overflowing at a write operartion, the oldest data
   * will be overwritten.
   */
  SharedRingBuffer(object_id_t objectId, uint8_t* buffer, const size_t size, bool overwriteOld,
                   size_t maxExcessBytes);

  virtual ~SharedRingBuffer();

  /**
   * @brief    This function can be used to add an optional FIFO to the class
   * @details
   * This FIFO will be allocated in the initialize function (and will
   * have a fixed maximum size after that). It can be used to store
   * values like packet sizes, for example for a shared ring buffer
   * used by producer/consumer tasks.
   */
  void setToUseReceiveSizeFIFO(size_t fifoDepth);

  /**
   * Unless a read-only constant value is read, all operations on the
   * shared ring buffer should be protected by calling this function.
   * @param timeoutType
   * @param timeout
   * @return
   */
  virtual ReturnValue_t lockRingBufferMutex(MutexIF::TimeoutType timeoutType, dur_millis_t timeout);
  /**
   * Any locked mutex also has to be unlocked, otherwise, access to the
   * shared ring buffer will be blocked.
   * @return
   */
  virtual ReturnValue_t unlockRingBufferMutex();

  /**
   * The mutex handle can be accessed directly, for example to perform
   * the lock with the #MutexGuard for a RAII compliant lock operation.
   * @return
   */
  MutexIF* getMutexHandle() const;

  ReturnValue_t initialize() override;

  /**
   * If the shared ring buffer was configured to have a sizes FIFO, a handle
   * to that FIFO can be retrieved with this function.
   * Do not forget to protect access with a lock if required!
   * @return
   */
  DynamicFIFO<size_t>* getReceiveSizesFIFO();

 private:
  MutexIF* mutex = nullptr;

  size_t fifoDepth = 0;
  DynamicFIFO<size_t>* receiveSizesFIFO = nullptr;
};

#endif /* FSFW_CONTAINER_SHAREDRINGBUFFER_H_ */

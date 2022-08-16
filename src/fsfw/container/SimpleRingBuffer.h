#ifndef FSFW_CONTAINER_SIMPLERINGBUFFER_H_
#define FSFW_CONTAINER_SIMPLERINGBUFFER_H_

#include <cstddef>

#include "RingBufferBase.h"

/**
 * @brief     Circular buffer implementation, useful for buffering into data streams.
 * @details
 * Note that the deleteData() has to be called to increment the read pointer.
 * This class allocated dynamically, so
 * @ingroup containers
 */
class SimpleRingBuffer : public RingBufferBase<> {
 public:
  /**
   * This constructor allocates a new internal buffer with the supplied size.
   *
   * @param size
   * @param overwriteOld If the ring buffer is overflowing at a write
   * operation, the oldest data will be overwritten.
   * @param maxExcessBytes These additional bytes will be allocated in addition
   * to the specified size to accommodate continuous write operations
   * with getFreeElement.
   *
   */
  SimpleRingBuffer(const size_t size, bool overwriteOld, size_t maxExcessBytes = 0);
  /**
   * This constructor takes an external buffer with the specified size.
   * @param buffer
   * @param size
   * @param overwriteOld
   * If the ring buffer is overflowing at a write operation, the oldest data
   * will be overwritten.
   * @param maxExcessBytes
   * If the buffer can accommodate additional bytes for contiguous write
   * operations with getFreeElement, this is the maximum allowed additional
   * size
   */
  SimpleRingBuffer(uint8_t* buffer, const size_t size, bool overwriteOld,
                   size_t maxExcessBytes = 0);

  virtual ~SimpleRingBuffer();

  /**
   * Write to circular buffer and increment write pointer by amount.
   * @param data
   * @param amount
   * @return -@c returnvalue::OK if write operation was successful
   * -@c returnvalue::FAILED if
   */
  ReturnValue_t writeData(const uint8_t* data, size_t amount);

  /**
   * Returns a pointer to a free element. If the remaining buffer is
   * not large enough, the data will be written past the actual size
   * and the amount of excess bytes will be cached. This function
   * does not increment the write pointer!
   * @param writePointer Pointer to a pointer which can be used to write
   * contiguous blocks into the ring buffer
   * @param amount
   * @return
   */
  ReturnValue_t getFreeElement(uint8_t** writePointer, size_t amount);

  /**
   * This increments the write pointer and also copies the excess bytes
   * to the beginning. It should be called if the write operation
   * conducted after calling getFreeElement() was performed.
   * @return
   */
  void confirmBytesWritten(size_t amount);

  virtual size_t getExcessBytes() const;
  /**
   * Helper functions which moves any excess bytes to the start
   * of the ring buffer.
   * @return
   */
  virtual void moveExcessBytesToStart();

  /**
   * Read from circular buffer at read pointer.
   * @param data
   * @param amount
   * @param incrementReadPtr
   * If this is set to true, the read pointer will be incremented.
   * If readRemaining is set to true, the read pointer will be incremented
   * accordingly.
   * @param readRemaining
   * If this is set to true, the data will be read even if the amount
   * specified exceeds the read data available.
   * @param trueAmount [out]
   * If readRemaining was set to true, the true amount read will be assigned
   * to the passed value.
   * @return
   * - @c returnvalue::OK if data was read successfully
   * - @c returnvalue::FAILED if not enough data was available and readRemaining
   *      was set to false.
   */
  ReturnValue_t readData(uint8_t* data, size_t amount, bool incrementReadPtr = false,
                         bool readRemaining = false, size_t* trueAmount = nullptr);

  /**
   * Delete data by incrementing read pointer.
   * @param amount
   * @param deleteRemaining
   * If the amount specified is larger than the remaining size to read and this
   * is set to true, the remaining amount will be deleted as well
   * @param trueAmount [out]
   * If deleteRemaining was set to true, the amount deleted will be assigned
   * to the passed value.
   * @return
   */
  ReturnValue_t deleteData(size_t amount, bool deleteRemaining = false,
                           size_t* trueAmount = nullptr);

 private:
  static const uint8_t READ_PTR = 0;
  uint8_t* buffer = nullptr;
  size_t maxExcessBytes;
  size_t excessBytes = 0;
};

#endif /* FSFW_CONTAINER_SIMPLERINGBUFFER_H_ */

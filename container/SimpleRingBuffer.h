#ifndef FRAMEWORK_CONTAINER_SIMPLERINGBUFFER_H_
#define FRAMEWORK_CONTAINER_SIMPLERINGBUFFER_H_

#include <framework/container/RingBufferBase.h>
#include <cstddef>

/**
 * @brief 	Circular buffer implementation, useful for buffering
 *          into data streams.
 * @details
 * Note that the deleteData() has to be called to increment the read pointer.
 * This class allocated dynamically, so
 * @ingroup containers
 */
class SimpleRingBuffer: public RingBufferBase<> {
public:
    /**
     * This constructor allocates a new internal buffer with the supplied size.
     * @param size
     * @param overwriteOld
     * If the ring buffer is overflowing at a write operartion, the oldest data
     * will be overwritten.
     */
	SimpleRingBuffer(const size_t size, bool overwriteOld);
	/**
	 * This constructor takes an external buffer with the specified size.
	 * @param buffer
	 * @param size
	 * @param overwriteOld
	 * If the ring buffer is overflowing at a write operartion, the oldest data
     * will be overwritten.
	 */
	SimpleRingBuffer(uint8_t* buffer, const size_t size, bool overwriteOld);

	virtual ~SimpleRingBuffer();

	/**
	 * Write to circular buffer and increment write pointer by amount.
	 * @param data
	 * @param amount
	 * @return -@c RETURN_OK if write operation was successfull
	 * -@c RETURN_FAILED if
	 */
	ReturnValue_t writeData(const uint8_t* data, size_t amount);

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
	 * - @c RETURN_OK if data was read successfully
	 * - @c RETURN_FAILED if not enough data was available and readRemaining
	 *      was set to false.
	 */
	ReturnValue_t readData(uint8_t* data, size_t amount,
			bool incrementReadPtr = false, bool readRemaining = false,
			size_t* trueAmount = nullptr);

	/**
	 * Delete data by incrementing read pointer.
	 * @param amount
	 * @param deleteRemaining
	 * If the amount specified is larger than the remaing size to read and this
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
};

#endif /* FRAMEWORK_CONTAINER_SIMPLERINGBUFFER_H_ */


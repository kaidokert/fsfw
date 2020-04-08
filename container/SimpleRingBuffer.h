#ifndef FRAMEWORK_CONTAINER_SIMPLERINGBUFFER_H_
#define FRAMEWORK_CONTAINER_SIMPLERINGBUFFER_H_

#include <framework/container/RingBufferBase.h>
#include <stddef.h>

/**
 * @brief 	Circular buffer implementation, useful for buffering into data streams.
 * @details Note that the deleteData() has to be called to increment the read pointer
 * @ingroup containers
 */
class SimpleRingBuffer: public RingBufferBase<> {
public:
	SimpleRingBuffer(uint32_t size, bool overwriteOld);
	virtual ~SimpleRingBuffer();

	/**
	 * Write to circular buffer and increment write pointer by amount
	 * @param data
	 * @param amount
	 * @return
	 */
	ReturnValue_t writeData(const uint8_t* data, uint32_t amount);

	/**
	 * Read from circular buffer at read pointer
	 * @param data
	 * @param amount
	 * @param readRemaining
	 * @param trueAmount
	 * @return
	 */
	ReturnValue_t readData(uint8_t* data, uint32_t amount, bool readRemaining = false, uint32_t* trueAmount = NULL);

	/**
	 * Delete data starting by incrementing read pointer
	 * @param amount
	 * @param deleteRemaining
	 * @param trueAmount
	 * @return
	 */
	ReturnValue_t deleteData(uint32_t amount, bool deleteRemaining = false, uint32_t* trueAmount = NULL);
private:
//	static const uint8_t TEMP_READ_PTR = 1;
	static const uint8_t READ_PTR = 0;
	uint8_t* buffer;
};

#endif /* FRAMEWORK_CONTAINER_SIMPLERINGBUFFER_H_ */


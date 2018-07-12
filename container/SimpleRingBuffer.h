#ifndef FRAMEWORK_CONTAINER_SIMPLERINGBUFFER_H_
#define FRAMEWORK_CONTAINER_SIMPLERINGBUFFER_H_

#include <framework/container/RingBufferBase.h>
#include <stddef.h>

class SimpleRingBuffer: public RingBufferBase<> {
public:
	SimpleRingBuffer(uint32_t size, bool overwriteOld);
	virtual ~SimpleRingBuffer();
	ReturnValue_t writeData(const uint8_t* data, uint32_t amount);
	ReturnValue_t readData(uint8_t* data, uint32_t amount, bool readRemaining = false, uint32_t* trueAmount = NULL);
	ReturnValue_t deleteData(uint32_t amount, bool deleteRemaining = false, uint32_t* trueAmount = NULL);
private:
//	static const uint8_t TEMP_READ_PTR = 1;
	static const uint8_t READ_PTR = 0;
	uint8_t* buffer;
};

#endif /* FRAMEWORK_CONTAINER_SIMPLERINGBUFFER_H_ */


#ifndef FRAMEWORK_CONTAINER_RINGBUFFERBASE_H_
#define FRAMEWORK_CONTAINER_RINGBUFFERBASE_H_

#include "../returnvalues/HasReturnvaluesIF.h"

template<uint8_t N_READ_PTRS = 1>
class RingBufferBase {
public:
	RingBufferBase(uint32_t startAddress, uint32_t size, bool overwriteOld) :
			start(startAddress), write(startAddress), size(size), overwriteOld(overwriteOld) {
		for (uint8_t count = 0; count < N_READ_PTRS; count++) {
			read[count] = startAddress;
		}
	}
	ReturnValue_t readData(uint32_t amount, uint8_t n = 0) {
		if (availableReadData(n) >= amount) {
			incrementRead(amount, n);
			return HasReturnvaluesIF::RETURN_OK;
		} else {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
	}
	ReturnValue_t writeData(uint32_t amount) {
		if (availableWriteSpace() >= amount || overwriteOld) {
			incrementWrite(amount);
			return HasReturnvaluesIF::RETURN_OK;
		} else {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
	}
	uint32_t availableReadData(uint8_t n = 0) const {
		return ((write + size) - read[n]) % size;
	}
	uint32_t availableWriteSpace(uint8_t n = 0) const  {
		//One less to avoid ambiguous full/empty problem.
		return (((read[n] + size) - write - 1) % size);
	}
	bool isFull(uint8_t n = 0) {
		return (availableWriteSpace(n) == 0);
	}
	bool isEmpty(uint8_t n = 0) {
		return (availableReadData(n) == 0);
	}
	virtual ~RingBufferBase() {

	}
	uint32_t getRead(uint8_t n = 0) const {
		return read[n];
	}
	void setRead(uint32_t read, uint8_t n = 0) {
		if (read >= start && read < (start+size)) {
			this->read[n] = read;
		}
	}
	uint32_t getWrite() const {
		return write;
	}
	void setWrite(uint32_t write) {
		this->write = write;
	}
	void clear() {
		write = start;
		for (uint8_t count = 0; count < N_READ_PTRS; count++) {
			read[count] = start;
		}
	}
	uint32_t writeTillWrap() {
		return (start + size) - write;
	}
	uint32_t readTillWrap(uint8_t n = 0) {
		return (start + size) - read[n];
	}
	uint32_t getStart() const {
		return start;
	}
	bool overwritesOld() const {
		return overwriteOld;
	}
	uint32_t maxSize() const {
		return size - 1;
	}
protected:
	const uint32_t start;
	uint32_t write;
	uint32_t read[N_READ_PTRS];
	const uint32_t size;
	const bool overwriteOld;
	void incrementWrite(uint32_t amount) {
		write = ((write + amount - start) % size) + start;
	}
	void incrementRead(uint32_t amount, uint8_t n = 0) {
		read[n] = ((read[n] + amount - start) % size) + start;
	}
};

#endif /* FRAMEWORK_CONTAINER_RINGBUFFERBASE_H_ */

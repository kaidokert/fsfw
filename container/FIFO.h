#ifndef FIFO_H_
#define FIFO_H_

#include <framework/returnvalues/HasReturnvaluesIF.h>

/**
 * @brief Simple First-In-First-Out data structure
 * @tparam T Entry Type
 * @tparam capacity Maximum capacity
 */
template<typename T, uint8_t capacity>
class FIFO {
private:
	uint8_t readIndex, writeIndex, currentSize;
	T data[capacity];

	uint8_t next(uint8_t current) {
		++current;
		if (current == capacity) {
			current = 0;
		}
		return current;
	}
public:
	FIFO() :
			readIndex(0), writeIndex(0), currentSize(0) {
	}

	bool empty() {
		return (currentSize == 0);
	}

	bool full() {
		return (currentSize == capacity);
	}

	uint8_t size(){
		return currentSize;
	}

	ReturnValue_t insert(T value) {
		if (full()) {
			return FULL;
		} else {
			data[writeIndex] = value;
			writeIndex = next(writeIndex);
			++currentSize;
			return HasReturnvaluesIF::RETURN_OK;
		}
	}

	ReturnValue_t retrieve(T *value) {
		if (empty()) {
			return EMPTY;
		} else {
			*value = data[readIndex];
			readIndex = next(readIndex);
			--currentSize;
			return HasReturnvaluesIF::RETURN_OK;
		}
	}
	static const uint8_t INTERFACE_ID = CLASS_ID::FIFO_CLASS;
	static const ReturnValue_t FULL = MAKE_RETURN_CODE(1);
	static const ReturnValue_t EMPTY = MAKE_RETURN_CODE(2);
};

#endif /* FIFO_H_ */

#ifndef FRAMEWORK_CONTAINER_FIFOBASE_H_
#define FRAMEWORK_CONTAINER_FIFOBASE_H_

#include "../returnvalues/HasReturnvaluesIF.h"
#include <cstddef>
#include <cstring>

template <typename T>
class FIFOBase {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::FIFO_CLASS;
	static const ReturnValue_t FULL = MAKE_RETURN_CODE(1);
	static const ReturnValue_t EMPTY = MAKE_RETURN_CODE(2);

	/** Default ctor, takes pointer to first entry of underlying container
	 *  and maximum capacity */
	FIFOBase(T* values, const size_t maxCapacity);

	/**
	 * Insert value into FIFO
	 * @param value
	 * @return
	 */
	ReturnValue_t insert(T value);
	/**
	 * Retrieve item from FIFO. This removes the item from the FIFO.
	 * @param value
	 * @return
	 */
	ReturnValue_t retrieve(T *value);
	/**
	 * Retrieve item from FIFO without removing it from FIFO.
	 * @param value
	 * @return
	 */
	ReturnValue_t peek(T * value);
	/**
	 * Remove item from FIFO.
	 * @return
	 */
	ReturnValue_t pop();

	bool empty();
	bool full();
	size_t size();


	size_t getMaxCapacity() const;

protected:
	void setData(T* data);
	size_t maxCapacity = 0;

	T* values;

	size_t readIndex = 0;
	size_t writeIndex = 0;
	size_t currentSize = 0;

	size_t next(size_t current);
};

#include "../container/FIFOBase.tpp"

#endif /* FRAMEWORK_CONTAINER_FIFOBASE_H_ */

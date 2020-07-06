#ifndef FRAMEWORK_CONTAINER_FIFOBASE_TPP_
#define FRAMEWORK_CONTAINER_FIFOBASE_TPP_

#ifndef FRAMEWORK_CONTAINER_FIFOBASE_H_
#error Include FIFOBase.h before FIFOBase.tpp!
#endif

template<typename T>
inline FIFOBase<T>::FIFOBase(T* values, const size_t maxCapacity):
		values(values), maxCapacity(maxCapacity) {};

template<typename T>
inline ReturnValue_t FIFOBase<T>::insert(T value) {
	if (full()) {
		return FULL;
	} else {
		values[writeIndex] = value;
		writeIndex = next(writeIndex);
		++currentSize;
		return HasReturnvaluesIF::RETURN_OK;
	}
};

template<typename T>
inline ReturnValue_t FIFOBase<T>::retrieve(T* value) {
	if (empty()) {
		return EMPTY;
	} else {
		*value = values[readIndex];
		readIndex = next(readIndex);
		--currentSize;
		return HasReturnvaluesIF::RETURN_OK;
	}
};

template<typename T>
inline ReturnValue_t FIFOBase<T>::peek(T* value) {
	if(empty()) {
		return EMPTY;
	} else {
		*value = values[readIndex];
		return HasReturnvaluesIF::RETURN_OK;
	}
};

template<typename T>
inline ReturnValue_t FIFOBase<T>::pop() {
	T value;
	return this->retrieve(&value);
};

template<typename T>
inline bool FIFOBase<T>::empty() {
	return (currentSize == 0);
};

template<typename T>
inline bool FIFOBase<T>::full() {
	return (currentSize == maxCapacity);
}

template<typename T>
inline size_t FIFOBase<T>::size() {
	return currentSize;
}

template<typename T>
inline size_t FIFOBase<T>::next(size_t current) {
	++current;
	if (current == maxCapacity) {
		current = 0;
	}
	return current;
}

#endif

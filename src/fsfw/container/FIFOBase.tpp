#ifndef FSFW_CONTAINER_FIFOBASE_TPP_
#define FSFW_CONTAINER_FIFOBASE_TPP_

#ifndef FSFW_CONTAINER_FIFOBASE_H_
#error Include FIFOBase.h before FIFOBase.tpp!
#endif

template <typename T>
inline FIFOBase<T>::FIFOBase(T* values, const size_t maxCapacity)
    : maxCapacity(maxCapacity), values(values){};

template <typename T>
inline ReturnValue_t FIFOBase<T>::insert(T value) {
  if (full()) {
    return FULL;
  } else {
    values[writeIndex] = value;
    writeIndex = next(writeIndex);
    ++currentSize;
    return returnvalue::OK;
  }
};

template <typename T>
inline ReturnValue_t FIFOBase<T>::retrieve(T* value) {
  if (empty()) {
    return EMPTY;
  } else {
    if (value == nullptr) {
      return returnvalue::FAILED;
    }
    *value = values[readIndex];
    readIndex = next(readIndex);
    --currentSize;
    return returnvalue::OK;
  }
};

template <typename T>
inline ReturnValue_t FIFOBase<T>::peek(T* value) {
  if (empty()) {
    return EMPTY;
  } else {
    if (value == nullptr) {
      return returnvalue::FAILED;
    }
    *value = values[readIndex];
    return returnvalue::OK;
  }
};

template <typename T>
inline ReturnValue_t FIFOBase<T>::pop() {
  T value;
  return this->retrieve(&value);
};

template <typename T>
inline bool FIFOBase<T>::empty() {
  return (currentSize == 0);
};

template <typename T>
inline bool FIFOBase<T>::full() {
  return (currentSize == maxCapacity);
}

template <typename T>
inline size_t FIFOBase<T>::size() {
  return currentSize;
}

template <typename T>
inline size_t FIFOBase<T>::next(size_t current) {
  ++current;
  if (current == maxCapacity) {
    current = 0;
  }
  return current;
}

template <typename T>
inline size_t FIFOBase<T>::getMaxCapacity() const {
  return maxCapacity;
}

template <typename T>
inline void FIFOBase<T>::setContainer(T* data) {
  this->values = data;
}

#endif

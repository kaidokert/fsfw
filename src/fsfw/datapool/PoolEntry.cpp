#include "fsfw/datapool/PoolEntry.h"

#include <algorithm>
#include <cstring>

#include "fsfw/globalfunctions/arrayprinter.h"
#include "fsfw/serviceinterface/ServiceInterface.h"

template <typename T>
PoolEntry<T>::PoolEntry(uint8_t len, bool setValid) : length(len), valid(setValid) {
  this->address = new T[this->length]();
  std::memset(this->address, 0, this->getByteSize());
}

template <typename T>
PoolEntry<T>::PoolEntry(std::initializer_list<T> initValues, bool setValid)
    : length(static_cast<uint8_t>(initValues.size())), valid(setValid) {
  this->address = new T[this->length]();
  if (initValues.size() > 0) {
    std::copy(initValues.begin(), initValues.end(), this->address);
  }
}

template <typename T>
PoolEntry<T>::PoolEntry(const T* initValue, uint8_t setLength, bool setValid)
    : length(setLength), valid(setValid) {
  this->address = new T[this->length]();
  if (initValue != nullptr) {
    std::memcpy(this->address, initValue, this->getByteSize());
  }
}

// As the data pool is global, this dtor is only be called on program exit.
// Warning! Never copy pool entries!
template <typename T>
PoolEntry<T>::~PoolEntry() {
  delete[] this->address;
}

template <typename T>
uint16_t PoolEntry<T>::getByteSize() {
  return (sizeof(T) * this->length);
}

template <typename T>
uint8_t PoolEntry<T>::getSize() {
  return this->length;
}

template <typename T>
void* PoolEntry<T>::getRawData() {
  return this->address;
}

template <typename T>
void PoolEntry<T>::setValid(bool isValid) {
  this->valid = isValid;
}

template <typename T>
bool PoolEntry<T>::getValid() {
  return valid;
}

template <typename T>
void PoolEntry<T>::print() {
  const char* validString = nullptr;
  if (valid) {
    validString = "Valid";
  } else {
    validString = "Invalid";
  }
#if FSFW_CPP_OSTREAM_ENABLED == 1
  sif::info << "PoolEntry information." << std::endl;
  sif::info << "PoolEntry validity: " << validString << std::endl;
#else
  sif::printInfo("PoolEntry information.\n");
  sif::printInfo("PoolEntry validity: %s\n", validString);
#endif
  arrayprinter::print(reinterpret_cast<uint8_t*>(address), getByteSize());
}

template <typename T>
inline T* PoolEntry<T>::getDataPtr() {
  return this->address;
}

template <typename T>
Type PoolEntry<T>::getType() {
  return PodTypeConversion<T>::type;
}

template class PoolEntry<uint8_t>;
template class PoolEntry<uint16_t>;
template class PoolEntry<uint32_t>;
template class PoolEntry<uint64_t>;
template class PoolEntry<int8_t>;
template class PoolEntry<int16_t>;
template class PoolEntry<int32_t>;
template class PoolEntry<int64_t>;
template class PoolEntry<float>;
template class PoolEntry<double>;

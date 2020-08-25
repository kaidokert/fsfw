#include "PoolEntry.h"
#include "../serviceinterface/ServiceInterfaceStream.h"
#include "../globalfunctions/arrayprinter.h"
#include <cstring>

template <typename T>
PoolEntry<T>::PoolEntry(std::initializer_list<T> initValue, uint8_t setLength,
		bool setValid ) : length(setLength), valid(setValid) {
	this->address = new T[this->length];
	if(initValue.size() == 0) {
		std::memset(this->address, 0, this->getByteSize());
	}
	else if (initValue.size() != setLength){
		sif::warning << "PoolEntry: setLength is not equal to initializer list"
				"length! Performing zero initialization with given setLength"
				<< std::endl;
		std::memset(this->address, 0, this->getByteSize());
	}
	else {
		std::copy(initValue.begin(), initValue.end(), this->address);
	}
}

template <typename T>
PoolEntry<T>::PoolEntry( T* initValue, uint8_t setLength, bool setValid ) :
		length(setLength), valid(setValid) {
	this->address = new T[this->length];
	if (initValue != nullptr) {
		std::memcpy(this->address, initValue, this->getByteSize() );
	} else {
		std::memset(this->address, 0, this->getByteSize() );
	}
}

//As the data pool is global, this dtor is only be called on program exit.
//Warning! Never copy pool entries!
template <typename T>
PoolEntry<T>::~PoolEntry() {
	delete[] this->address;
}

template <typename T>
uint16_t PoolEntry<T>::getByteSize() {
	return ( sizeof(T) * this->length );
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
	 sif::debug << "Pool Entry Validity: " <<
			 (this->valid? " (valid) " : " (invalid) ") << std::endl;
	arrayprinter::print(reinterpret_cast<uint8_t*>(address), length);
	sif::debug << std::dec << std::endl;
}

template<typename T>
Type PoolEntry<T>::getType() {
	return PodTypeConversion<T>::type;
}

template class PoolEntry<uint8_t>;
template class PoolEntry<uint16_t>;
template class PoolEntry<uint32_t>;
template class PoolEntry<int8_t>;
template class PoolEntry<int16_t>;
template class PoolEntry<int32_t>;
template class PoolEntry<float>;
template class PoolEntry<double>;

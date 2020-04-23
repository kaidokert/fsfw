#include <framework/datapool/PoolEntry.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>

template <typename T>
PoolEntry<T>::PoolEntry(std::initializer_list<T> initValue, uint8_t set_length,
		uint8_t set_valid ) : length(set_length), valid(set_valid) {
	this->address = new T[this->length];
	if(initValue.size() == 0) {
		memset(this->address, 0, this->getByteSize());
	}
	else {
		memcpy(this->address, initValue.begin(), this->getByteSize());
	}
}

template <typename T>
PoolEntry<T>::PoolEntry( T* initValue, uint8_t set_length, uint8_t set_valid ) :
		length(set_length), valid(set_valid) {
	this->address = new T[this->length];
	if (initValue != NULL) {
		memcpy(this->address, initValue, this->getByteSize() );
	} else {
		memset(this->address, 0, this->getByteSize() );
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
void PoolEntry<T>::setValid( uint8_t isValid ) {
	this->valid = isValid;
}

template <typename T>
uint8_t PoolEntry<T>::getValid() {
	return valid;
}

template <typename T>
void PoolEntry<T>::print() {
	for (uint8_t size = 0; size < this->length; size++ ) {
		sif::debug << "| " << std::hex << (double)this->address[size]
		           << (this->valid? " (valid) " : " (invalid) ");
	}
	sif::debug << std::dec << std::endl;
}

template<typename T>
Type PoolEntry<T>::getType() {
	return PodTypeConversion<T>::type;
}

template class PoolEntry<bool>;
template class PoolEntry<uint8_t>;
template class PoolEntry<uint16_t>;
template class PoolEntry<uint32_t>;
template class PoolEntry<int8_t>;
template class PoolEntry<int16_t>;
template class PoolEntry<int32_t>;
template class PoolEntry<float>;
template class PoolEntry<double>;

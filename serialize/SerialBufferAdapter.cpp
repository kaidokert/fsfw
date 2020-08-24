#include "SerialBufferAdapter.h"
#include <cstring>



template<typename T>
SerialBufferAdapter<T>::SerialBufferAdapter(const uint8_t* buffer,
		T bufferLength, bool serializeLenght) :
		serializeLength(serializeLenght), constBuffer(buffer), buffer(NULL), bufferLength(
				bufferLength) {
}

template<typename T>
SerialBufferAdapter<T>::SerialBufferAdapter(uint8_t* buffer, T bufferLength,
		bool serializeLenght) :
		serializeLength(serializeLenght), constBuffer(NULL), buffer(buffer), bufferLength(
				bufferLength) {
}

template<typename T>
SerialBufferAdapter<T>::~SerialBufferAdapter() {
}

template<typename T>
ReturnValue_t SerialBufferAdapter<T>::serialize(uint8_t** buffer, size_t* size,
		size_t maxSize, Endianness streamEndianness) const {
	uint32_t serializedLength = bufferLength;
	if (serializeLength) {
		serializedLength += SerializeAdapter::getSerializedSize(
				&bufferLength);
	}
	if (*size + serializedLength > maxSize) {
		return BUFFER_TOO_SHORT;
	} else {
		if (serializeLength) {
			SerializeAdapter::serialize(&bufferLength, buffer, size,
					maxSize, streamEndianness);
		}
		if (this->constBuffer != NULL) {
			memcpy(*buffer, this->constBuffer, bufferLength);
		} else if (this->buffer != NULL) {
			memcpy(*buffer, this->buffer, bufferLength);
		} else {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
		*size += bufferLength;
		(*buffer) += bufferLength;
		return HasReturnvaluesIF::RETURN_OK;
	}
}

template<typename T>
size_t SerialBufferAdapter<T>::getSerializedSize() const {
	if (serializeLength) {
		return bufferLength + SerializeAdapter::getSerializedSize(&bufferLength);
	} else {
		return bufferLength;
	}
}
template<typename T>
ReturnValue_t SerialBufferAdapter<T>::deSerialize(const uint8_t** buffer,
		size_t* size, Endianness streamEndianness) {
	//TODO Ignores Endian flag!
	if (buffer != NULL) {
		if(serializeLength){
			T serializedSize = SerializeAdapter::getSerializedSize(
					&bufferLength);
			if((*size - bufferLength - serializedSize) >= 0){
				*buffer +=  serializedSize;
				*size -= serializedSize;
			}else{
				return STREAM_TOO_SHORT;
			}
		}
		//No Else If, go on with buffer
		if (*size - bufferLength >= 0) {
			*size -= bufferLength;
			memcpy(this->buffer, *buffer, bufferLength);
			(*buffer) += bufferLength;
			return HasReturnvaluesIF::RETURN_OK;
		} else {
			return STREAM_TOO_SHORT;
		}
	} else {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}


//forward Template declaration for linker
template class SerialBufferAdapter<uint8_t>;
template class SerialBufferAdapter<uint16_t>;
template class SerialBufferAdapter<uint32_t>;

